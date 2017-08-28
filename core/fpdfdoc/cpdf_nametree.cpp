// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_nametree.h"

#include <utility>
#include <vector>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"

namespace {

const int nMaxRecursion = 32;

std::pair<CFX_WideString, CFX_WideString> GetNodeLimitsMaybeSwap(
    CPDF_Array* pLimits) {
  ASSERT(pLimits);
  CFX_WideString csLeft = pLimits->GetUnicodeTextAt(0);
  CFX_WideString csRight = pLimits->GetUnicodeTextAt(1);
  // If the lower limit is greater than the upper limit, swap them.
  if (csLeft.Compare(csRight) > 0) {
    pLimits->SetNewAt<CPDF_String>(0, csRight);
    pLimits->SetNewAt<CPDF_String>(1, csLeft);
    csLeft = pLimits->GetUnicodeTextAt(0);
    csRight = pLimits->GetUnicodeTextAt(1);
  }
  return {csLeft, csRight};
}

// Get the limit arrays that leaf array |pFind| is under in the tree with root
// |pNode|. |pLimits| will hold all the limit arrays from the leaf up to before
// the root. Return true if successful.
bool GetNodeAncestorsLimits(const CPDF_Dictionary* pNode,
                            const CPDF_Array* pFind,
                            int nLevel,
                            std::vector<CPDF_Array*>* pLimits) {
  if (nLevel > nMaxRecursion)
    return false;

  if (pNode->GetArrayFor("Names") == pFind) {
    pLimits->push_back(pNode->GetArrayFor("Limits"));
    return true;
  }

  CPDF_Array* pKids = pNode->GetArrayFor("Kids");
  if (!pKids)
    return false;

  for (size_t i = 0; i < pKids->GetCount(); ++i) {
    CPDF_Dictionary* pKid = pKids->GetDictAt(i);
    if (!pKid)
      continue;

    if (GetNodeAncestorsLimits(pKid, pFind, nLevel + 1, pLimits)) {
      pLimits->push_back(pNode->GetArrayFor("Limits"));
      return true;
    }
  }
  return false;
}

// Upon the deletion of |csName| from leaf array |pFind|, update the ancestors
// of |pFind|. Specifically, the limits of |pFind|'s ancestors will be updated
// if needed, and any ancestors that are now empty will be removed.
bool UpdateNodesAndLimitsUponDeletion(CPDF_Dictionary* pNode,
                                      const CPDF_Array* pFind,
                                      const CFX_WideString& csName,
                                      int nLevel) {
  if (nLevel > nMaxRecursion)
    return false;

  CPDF_Array* pLimits = pNode->GetArrayFor("Limits");
  CFX_WideString csLeft;
  CFX_WideString csRight;
  if (pLimits)
    std::tie(csLeft, csRight) = GetNodeLimitsMaybeSwap(pLimits);

  CPDF_Array* pNames = pNode->GetArrayFor("Names");
  if (pNames) {
    if (pNames != pFind)
      return false;
    if (pNames->IsEmpty() || !pLimits)
      return true;
    if (csLeft != csName && csRight != csName)
      return true;

    // Since |csName| defines |pNode|'s limits, we need to loop through the
    // names to find the new lower and upper limits.
    CFX_WideString csNewLeft = csRight;
    CFX_WideString csNewRight = csLeft;
    for (size_t i = 0; i < pNames->GetCount() / 2; ++i) {
      CFX_WideString wsName = pNames->GetUnicodeTextAt(i * 2);
      if (wsName.Compare(csNewLeft) < 0)
        csNewLeft = wsName;
      if (wsName.Compare(csNewRight) > 0)
        csNewRight = wsName;
    }
    pLimits->SetNewAt<CPDF_String>(0, csNewLeft);
    pLimits->SetNewAt<CPDF_String>(1, csNewRight);
    return true;
  }

  CPDF_Array* pKids = pNode->GetArrayFor("Kids");
  if (!pKids)
    return false;

  // Loop through the kids to find the leaf array |pFind|.
  for (size_t i = 0; i < pKids->GetCount(); ++i) {
    CPDF_Dictionary* pKid = pKids->GetDictAt(i);
    if (!pKid)
      continue;
    if (!UpdateNodesAndLimitsUponDeletion(pKid, pFind, csName, nLevel + 1))
      continue;

    // Remove this child node if it's empty.
    if ((pKid->KeyExist("Names") && pKid->GetArrayFor("Names")->IsEmpty()) ||
        (pKid->KeyExist("Kids") && pKid->GetArrayFor("Kids")->IsEmpty())) {
      pKids->RemoveAt(i);
    }
    if (pKids->IsEmpty() || !pLimits)
      return true;
    if (csLeft != csName && csRight != csName)
      return true;

    // Since |csName| defines |pNode|'s limits, we need to loop through the
    // kids to find the new lower and upper limits.
    CFX_WideString csNewLeft = csRight;
    CFX_WideString csNewRight = csLeft;
    for (size_t j = 0; j < pKids->GetCount(); ++j) {
      CPDF_Array* pKidLimits = pKids->GetDictAt(j)->GetArrayFor("Limits");
      ASSERT(pKidLimits);
      if (pKidLimits->GetUnicodeTextAt(0).Compare(csNewLeft) < 0)
        csNewLeft = pKidLimits->GetUnicodeTextAt(0);
      if (pKidLimits->GetUnicodeTextAt(1).Compare(csNewRight) > 0)
        csNewRight = pKidLimits->GetUnicodeTextAt(1);
    }
    pLimits->SetNewAt<CPDF_String>(0, csNewLeft);
    pLimits->SetNewAt<CPDF_String>(1, csNewRight);
    return true;
  }
  return false;
}

// Search for |csName| in the tree with root |pNode|. If successful, return the
// value that |csName| points to; |nIndex| will be the index of |csName|,
// |ppFind| will be the leaf array that |csName| is found in, and |pFindIndex|
// will be the index of |csName| in |ppFind|. If |csName| is not found, |ppFind|
// will be the leaf array that |csName| should be added to, and |pFindIndex|
// will be the index that it should be added at.
CPDF_Object* SearchNameNode(CPDF_Dictionary* pNode,
                            const CFX_WideString& csName,
                            size_t& nIndex,
                            int nLevel,
                            CPDF_Array** ppFind,
                            int* pFindIndex) {
  if (nLevel > nMaxRecursion)
    return nullptr;

  CPDF_Array* pLimits = pNode->GetArrayFor("Limits");
  CPDF_Array* pNames = pNode->GetArrayFor("Names");
  if (pLimits) {
    CFX_WideString csLeft;
    CFX_WideString csRight;
    std::tie(csLeft, csRight) = GetNodeLimitsMaybeSwap(pLimits);
    // Skip this node if the name to look for is smaller than its lower limit.
    if (csName.Compare(csLeft) < 0)
      return nullptr;

    // Skip this node if the name to look for is greater than its higher limit,
    // and the node itself is a leaf node.
    if (csName.Compare(csRight) > 0 && pNames) {
      if (ppFind)
        *ppFind = pNames;
      if (pFindIndex)
        *pFindIndex = pNames->GetCount() / 2 - 1;

      return nullptr;
    }
  }

  // If the node is a leaf node, look for the name in its names array.
  if (pNames) {
    size_t dwCount = pNames->GetCount() / 2;
    for (size_t i = 0; i < dwCount; i++) {
      CFX_WideString csValue = pNames->GetUnicodeTextAt(i * 2);
      int32_t iCompare = csValue.Compare(csName);
      if (iCompare > 0)
        break;
      if (ppFind)
        *ppFind = pNames;
      if (pFindIndex)
        *pFindIndex = i;
      if (iCompare < 0)
        continue;

      nIndex += i;
      return pNames->GetDirectObjectAt(i * 2 + 1);
    }
    nIndex += dwCount;
    return nullptr;
  }

  // Search through the node's children.
  CPDF_Array* pKids = pNode->GetArrayFor("Kids");
  if (!pKids)
    return nullptr;

  for (size_t i = 0; i < pKids->GetCount(); i++) {
    CPDF_Dictionary* pKid = pKids->GetDictAt(i);
    if (!pKid)
      continue;

    CPDF_Object* pFound =
        SearchNameNode(pKid, csName, nIndex, nLevel + 1, ppFind, pFindIndex);
    if (pFound)
      return pFound;
  }
  return nullptr;
}

// Get the key-value pair at |nIndex| in the tree with root |pNode|. If
// successful, return the value object; |csName| will be the key, |ppFind|
// will be the leaf array that this pair is in, and |pFindIndex| will be the
// index of the pair in |pFind|.
CPDF_Object* SearchNameNode(CPDF_Dictionary* pNode,
                            size_t nIndex,
                            size_t& nCurIndex,
                            int nLevel,
                            CFX_WideString* csName,
                            CPDF_Array** ppFind,
                            int* pFindIndex) {
  if (nLevel > nMaxRecursion)
    return nullptr;

  CPDF_Array* pNames = pNode->GetArrayFor("Names");
  if (pNames) {
    size_t nCount = pNames->GetCount() / 2;
    if (nIndex >= nCurIndex + nCount) {
      nCurIndex += nCount;
      return nullptr;
    }
    if (ppFind)
      *ppFind = pNames;
    if (pFindIndex)
      *pFindIndex = nIndex - nCurIndex;

    *csName = pNames->GetUnicodeTextAt((nIndex - nCurIndex) * 2);
    return pNames->GetDirectObjectAt((nIndex - nCurIndex) * 2 + 1);
  }

  CPDF_Array* pKids = pNode->GetArrayFor("Kids");
  if (!pKids)
    return nullptr;

  for (size_t i = 0; i < pKids->GetCount(); i++) {
    CPDF_Dictionary* pKid = pKids->GetDictAt(i);
    if (!pKid)
      continue;
    CPDF_Object* pFound = SearchNameNode(pKid, nIndex, nCurIndex, nLevel + 1,
                                         csName, ppFind, pFindIndex);
    if (pFound)
      return pFound;
  }
  return nullptr;
}

// Get the total number of key-value pairs in the tree with root |pNode|.
size_t CountNames(CPDF_Dictionary* pNode, int nLevel = 0) {
  if (nLevel > nMaxRecursion)
    return 0;

  CPDF_Array* pNames = pNode->GetArrayFor("Names");
  if (pNames)
    return pNames->GetCount() / 2;

  CPDF_Array* pKids = pNode->GetArrayFor("Kids");
  if (!pKids)
    return 0;

  size_t nCount = 0;
  for (size_t i = 0; i < pKids->GetCount(); i++) {
    CPDF_Dictionary* pKid = pKids->GetDictAt(i);
    if (!pKid)
      continue;

    nCount += CountNames(pKid, nLevel + 1);
  }
  return nCount;
}

}  // namespace

CPDF_NameTree::CPDF_NameTree(CPDF_Dictionary* pRoot) : m_pRoot(pRoot) {}

CPDF_NameTree::CPDF_NameTree(CPDF_Document* pDoc,
                             const CFX_ByteString& category)
    : m_pRoot(nullptr) {
  const CPDF_Dictionary* pRoot = pDoc->GetRoot();
  if (!pRoot)
    return;

  CPDF_Dictionary* pNames = pRoot->GetDictFor("Names");
  if (!pNames)
    return;

  m_pRoot = pNames->GetDictFor(category);
}

CPDF_NameTree::~CPDF_NameTree() {}

size_t CPDF_NameTree::GetCount() const {
  return m_pRoot ? ::CountNames(m_pRoot.Get()) : 0;
}

int CPDF_NameTree::GetIndex(const CFX_WideString& csName) const {
  if (!m_pRoot)
    return -1;

  size_t nIndex = 0;
  if (!SearchNameNode(m_pRoot.Get(), csName, nIndex, 0, nullptr, nullptr))
    return -1;
  return nIndex;
}

bool CPDF_NameTree::AddValueAndName(std::unique_ptr<CPDF_Object> pObj,
                                    const CFX_WideString& name) {
  if (!m_pRoot)
    return false;

  size_t nIndex = 0;
  CPDF_Array* pFind = nullptr;
  int nFindIndex = -1;
  // Fail if the tree already contains this name or if the tree is too deep.
  if (SearchNameNode(m_pRoot.Get(), name, nIndex, 0, &pFind, &nFindIndex))
    return false;

  // If the returned |pFind| is a nullptr, then |name| is smaller than all
  // existing entries in the tree, and we did not find a leaf array to place
  // |name| into. We instead will find the leftmost leaf array in which to place
  // |name| and |pObj|.
  if (!pFind) {
    size_t nCurIndex = 0;
    CFX_WideString csName;
    SearchNameNode(m_pRoot.Get(), 0, nCurIndex, 0, &csName, &pFind, nullptr);
  }
  ASSERT(pFind);

  // Insert the name and the object into the leaf array found. Note that the
  // insertion position is right after the key-value pair returned by |index|.
  size_t nNameIndex = (nFindIndex + 1) * 2;
  size_t nValueIndex = nNameIndex + 1;
  pFind->InsertNewAt<CPDF_String>(nNameIndex, name);
  pFind->InsertAt(nValueIndex, std::move(pObj));

  // Expand the limits that the newly added name is under, if the name falls
  // outside of the limits of its leaf array or any arrays above it.
  std::vector<CPDF_Array*> pLimits;
  GetNodeAncestorsLimits(m_pRoot.Get(), pFind, 0, &pLimits);
  for (auto* pLimit : pLimits) {
    if (!pLimit)
      continue;

    if (name.Compare(pLimit->GetUnicodeTextAt(0)) < 0)
      pLimit->SetNewAt<CPDF_String>(0, name);

    if (name.Compare(pLimit->GetUnicodeTextAt(1)) > 0)
      pLimit->SetNewAt<CPDF_String>(1, name);
  }
  return true;
}

bool CPDF_NameTree::DeleteValueAndName(int nIndex) {
  if (!m_pRoot)
    return false;

  size_t nCurIndex = 0;
  CFX_WideString csName;
  CPDF_Array* pFind = nullptr;
  int nFindIndex = -1;
  // Fail if the tree does not contain |nIndex|.
  if (!SearchNameNode(m_pRoot.Get(), nIndex, nCurIndex, 0, &csName, &pFind,
                      &nFindIndex)) {
    return false;
  }

  // Remove the name and the object from the leaf array |pFind|.
  pFind->RemoveAt(nFindIndex * 2);
  pFind->RemoveAt(nFindIndex * 2);

  // Delete empty nodes and update the limits of |pFind|'s ancestors as needed.
  UpdateNodesAndLimitsUponDeletion(m_pRoot.Get(), pFind, csName, 0);
  return true;
}

CPDF_Object* CPDF_NameTree::LookupValueAndName(int nIndex,
                                               CFX_WideString* csName) const {
  csName->clear();
  if (!m_pRoot)
    return nullptr;

  size_t nCurIndex = 0;
  return SearchNameNode(m_pRoot.Get(), nIndex, nCurIndex, 0, csName, nullptr,
                        nullptr);
}

CPDF_Object* CPDF_NameTree::LookupValue(const CFX_WideString& csName) const {
  if (!m_pRoot)
    return nullptr;

  size_t nIndex = 0;
  return SearchNameNode(m_pRoot.Get(), csName, nIndex, 0, nullptr, nullptr);
}

CPDF_Array* CPDF_NameTree::LookupNamedDest(CPDF_Document* pDoc,
                                           const CFX_WideString& sName) {
  CPDF_Object* pValue = LookupValue(sName);
  if (!pValue) {
    CPDF_Dictionary* pDests = pDoc->GetRoot()->GetDictFor("Dests");
    if (!pDests)
      return nullptr;
    pValue = pDests->GetDirectObjectFor(PDF_EncodeText(sName));
  }
  if (!pValue)
    return nullptr;
  if (CPDF_Array* pArray = pValue->AsArray())
    return pArray;
  if (CPDF_Dictionary* pDict = pValue->AsDictionary())
    return pDict->GetArrayFor("D");
  return nullptr;
}
