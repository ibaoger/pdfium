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

bool SearchAddNameNode(CPDF_Dictionary* pNode,
                       const CFX_WideString& csName,
                       int nLevel,
                       CPDF_Array** ppFind,
                       int* pFindIndex) {
  if (nLevel > nMaxRecursion)
    return false;

  CPDF_Array* pLimits = pNode->GetArrayFor("Limits");
  if (pLimits) {
    CFX_WideString csLeft = pLimits->GetUnicodeTextAt(0);
    // If the lower limit is greater than the higher limit, swap them.
    if (csLeft.Compare(pLimits->GetUnicodeTextAt(1)) > 0)
      csLeft = pLimits->GetUnicodeTextAt(1);

    // Skip this node if the name to look for comes before this node.
    if (csName.Compare(csLeft) < 0)
      return true;
  }

  CPDF_Array* pNames = pNode->GetArrayFor("Names");
  if (pNames) {
    // Loop through the names array to compare with the name to be added.
    for (size_t i = 0; i < pNames->GetCount(); i += 2) {
      int32_t iCompare = pNames->GetUnicodeTextAt(i).Compare(csName);
      // Fail if this name tree already contains this name.
      if (iCompare == 0)
        return false;

      // Records the current position if the name has not been reached yet.
      if (iCompare < 0) {
        if (ppFind)
          *ppFind = pNames;

        if (pFindIndex)
          *pFindIndex = i;

        continue;
      }
      // Return if the name has passed.
      return true;
    }
    return true;
  }

  CPDF_Array* pKids = pNode->GetArrayFor("Kids");
  if (!pKids)
    return true;

  for (size_t i = 0; i < pKids->GetCount(); ++i) {
    CPDF_Dictionary* pKid = pKids->GetDictAt(i);
    if (!pKid)
      continue;

    // Search through the kids and propagate any failure back up.
    if (!SearchAddNameNode(pKid, csName, nLevel + 1, ppFind, pFindIndex))
      return false;
  }
  return true;
}

bool GetNodeLimits(const CPDF_Dictionary* pNode,
                   const CPDF_Array* pFind,
                   const CFX_WideString& csName,
                   int nLevel,
                   std::vector<CPDF_Array*>* pLimits) {
  if (nLevel > nMaxRecursion)
    return false;

  CPDF_Array* pCurLimits = pNode->GetArrayFor("Limits");
  if (pCurLimits) {
    CFX_WideString csLeft = pCurLimits->GetUnicodeTextAt(0);
    if (csLeft.Compare(pCurLimits->GetUnicodeTextAt(1)) > 0)
      csLeft = pCurLimits->GetUnicodeTextAt(1);

    if (csName.Compare(csLeft) < 0)
      return false;
  }

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

    if (GetNodeLimits(pKid, pFind, csName, nLevel + 1, pLimits)) {
      pLimits->push_back(pNode->GetArrayFor("Limits"));
      return true;
    }
  }
  return false;
}

CPDF_Object* SearchNameNode(CPDF_Dictionary* pNode,
                            const CFX_WideString& csName,
                            size_t& nIndex,
                            CPDF_Array** ppFind,
                            int nLevel = 0) {
  if (nLevel > nMaxRecursion)
    return nullptr;

  CPDF_Array* pLimits = pNode->GetArrayFor("Limits");
  if (pLimits) {
    CFX_WideString csLeft = pLimits->GetUnicodeTextAt(0);
    CFX_WideString csRight = pLimits->GetUnicodeTextAt(1);
    if (csLeft.Compare(csRight) > 0) {
      CFX_WideString csTmp = csRight;
      csRight = csLeft;
      csLeft = csTmp;
    }
    if (csName.Compare(csLeft) < 0 || csName.Compare(csRight) > 0) {
      return nullptr;
    }
  }

  CPDF_Array* pNames = pNode->GetArrayFor("Names");
  if (pNames) {
    size_t dwCount = pNames->GetCount() / 2;
    for (size_t i = 0; i < dwCount; i++) {
      CFX_WideString csValue = pNames->GetUnicodeTextAt(i * 2);
      int32_t iCompare = csValue.Compare(csName);
      if (iCompare <= 0) {
        if (ppFind)
          *ppFind = pNames;
        if (iCompare < 0)
          continue;
      } else {
        break;
      }
      nIndex += i;
      return pNames->GetDirectObjectAt(i * 2 + 1);
    }
    nIndex += dwCount;
    return nullptr;
  }

  CPDF_Array* pKids = pNode->GetArrayFor("Kids");
  if (!pKids)
    return nullptr;

  for (size_t i = 0; i < pKids->GetCount(); i++) {
    CPDF_Dictionary* pKid = pKids->GetDictAt(i);
    if (!pKid)
      continue;

    CPDF_Object* pFound =
        SearchNameNode(pKid, csName, nIndex, ppFind, nLevel + 1);
    if (pFound)
      return pFound;
  }
  return nullptr;
}

CPDF_Object* SearchNameNode(CPDF_Dictionary* pNode,
                            size_t nIndex,
                            size_t& nCurIndex,
                            CFX_WideString* csName,
                            CPDF_Array** ppFind,
                            int nLevel = 0) {
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
    CPDF_Object* pFound =
        SearchNameNode(pKid, nIndex, nCurIndex, csName, ppFind, nLevel + 1);
    if (pFound)
      return pFound;
  }
  return nullptr;
}

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
  CPDF_Dictionary* pRoot = pDoc->GetRoot();
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
  if (!SearchNameNode(m_pRoot.Get(), csName, nIndex, nullptr))
    return -1;
  return nIndex;
}

CPDF_Object* CPDF_NameTree::AddValueAndName(const CFX_WideString& name,
                                            std::unique_ptr<CPDF_Object> pObj) {
  if (!m_pRoot)
    return nullptr;

  CPDF_Array* pFind = nullptr;
  int index = -2;
  // Fail if the tree already contains this name or if the tree is too deep.
  if (!SearchAddNameNode(m_pRoot.Get(), name, 0, &pFind, &index))
    return nullptr;

  // Finds the leftmost names array if the new name will be the leftmost leaf.
  if (!pFind) {
    size_t nCurIndex = 0;
    CFX_WideString csName;
    SearchNameNode(m_pRoot.Get(), 0, nCurIndex, &csName, &pFind);
  }

  // Insert the name and the object into the names array found.
  ASSERT(pFind);
  CPDF_Object* pRet = pObj.get();
  pFind->InsertNewAt<CPDF_String>(index + 2, name);
  pFind->InsertAt(index + 3, std::move(pObj));

  // Expand the limits if needed.
  std::vector<CPDF_Array*> pLimits;
  GetNodeLimits(m_pRoot.Get(), pFind, name, 0, &pLimits);
  for (auto* pLimit : pLimits) {
    if (pLimit && name.Compare(pLimit->GetUnicodeTextAt(1)) > 0)
      pLimit->SetNewAt<CPDF_String>(1, name);
  }
  return pRet;
}

CPDF_Object* CPDF_NameTree::LookupValueAndName(int nIndex,
                                               CFX_WideString* csName) const {
  *csName = CFX_WideString();
  if (!m_pRoot)
    return nullptr;

  size_t nCurIndex = 0;
  return SearchNameNode(m_pRoot.Get(), nIndex, nCurIndex, csName, nullptr);
}

CPDF_Object* CPDF_NameTree::LookupValue(const CFX_WideString& csName) const {
  if (!m_pRoot)
    return nullptr;

  size_t nIndex = 0;
  return SearchNameNode(m_pRoot.Get(), csName, nIndex, nullptr);
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
