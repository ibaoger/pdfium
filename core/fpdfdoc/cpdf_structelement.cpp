// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_structelement.h"

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfdoc/cpdf_structtree.h"
#include "third_party/base/stl_util.h"

CPDF_StructKid::CPDF_StructKid()
    : m_Type(Invalid),
      m_pDict(nullptr),
      m_PageObjNum(0),
      m_RefObjNum(0),
      m_ContentId(0) {}

CPDF_StructKid::CPDF_StructKid(const CPDF_StructKid& that) = default;

CPDF_StructKid::~CPDF_StructKid() = default;

CPDF_StructElement::CPDF_StructElement(CPDF_StructTree* pTree,
                                       CPDF_StructElement* pParent,
                                       CPDF_Dictionary* pDict)
    : m_pTree(pTree),
      m_pParent(pParent),
      m_pDict(pDict),
      m_Type(pDict->GetStringFor("S")),
      m_Title(pDict->GetStringFor("T")) {
  if (pTree->GetRoleMap()) {
    CFX_ByteString mapped = pTree->GetRoleMap()->GetStringFor(m_Type);
    if (!mapped.IsEmpty())
      m_Type = mapped;
  }
  LoadKids(pDict);
}

CPDF_StructElement::~CPDF_StructElement() = default;

int CPDF_StructElement::CountKids() const {
  return pdfium::CollectionSize<int>(m_Kids);
}

CPDF_StructElement* CPDF_StructElement::GetKidIfElement(int index) const {
  return m_Kids[index].m_Type == CPDF_StructKid::Element
             ? m_Kids[index].m_pElement.Get()
             : nullptr;
}

void CPDF_StructElement::LoadKids(CPDF_Dictionary* pDict) {
  CPDF_Object* pObj = pDict->GetObjectFor("Pg");
  uint32_t PageObjNum = 0;
  if (CPDF_Reference* pRef = ToReference(pObj))
    PageObjNum = pRef->GetRefObjNum();

  CPDF_Object* pKids = pDict->GetDirectObjectFor("K");
  if (!pKids)
    return;

  m_Kids.clear();
  if (CPDF_Array* pArray = pKids->AsArray()) {
    m_Kids.resize(pArray->GetCount());
    for (uint32_t i = 0; i < pArray->GetCount(); i++) {
      CPDF_Object* pKid = pArray->GetDirectObjectAt(i);
      LoadKid(PageObjNum, pKid, &m_Kids[i]);
    }
    return;
  }

  m_Kids.resize(1);
  LoadKid(PageObjNum, pKids, &m_Kids[0]);
}

void CPDF_StructElement::LoadKid(uint32_t PageObjNum,
                                 CPDF_Object* pKidObj,
                                 CPDF_StructKid* pKid) {
  pKid->m_Type = CPDF_StructKid::Invalid;
  if (!pKidObj)
    return;

  if (pKidObj->IsNumber()) {
    if (m_pTree->GetPage() && m_pTree->GetPage()->GetObjNum() != PageObjNum)
      return;

    pKid->m_Type = CPDF_StructKid::PageContent;
    pKid->m_ContentId = pKidObj->GetInteger();
    pKid->m_PageObjNum = PageObjNum;
    return;
  }

  CPDF_Dictionary* pKidDict = pKidObj->AsDictionary();
  if (!pKidDict)
    return;
  if (CPDF_Reference* pRef = ToReference(pKidDict->GetObjectFor("Pg")))
    PageObjNum = pRef->GetRefObjNum();

  CFX_ByteString type = pKidDict->GetStringFor("Type");
  if ((type == "MCR" || type == "OBJR") && m_pTree->GetPage() &&
      m_pTree->GetPage()->GetObjNum() != PageObjNum) {
    return;
  }

  if (type == "MCR") {
    pKid->m_Type = CPDF_StructKid::StreamContent;
    CPDF_Reference* pRef = ToReference(pKidDict->GetObjectFor("Stm"));
    pKid->m_RefObjNum = pRef ? pRef->GetRefObjNum() : 0;
    pKid->m_PageObjNum = PageObjNum;
    pKid->m_ContentId = pKidDict->GetIntegerFor("MCID");
    return;
  }

  if (type == "OBJR") {
    pKid->m_Type = CPDF_StructKid::Object;
    CPDF_Reference* pObj = ToReference(pKidDict->GetObjectFor("Obj"));
    pKid->m_RefObjNum = pObj ? pObj->GetRefObjNum() : 0;
    pKid->m_PageObjNum = PageObjNum;
    return;
  }

  pKid->m_Type = CPDF_StructKid::Element;
  pKid->m_pDict = pKidDict;
  if (m_pTree->GetPage()) {
    pKid->m_pElement = nullptr;
    return;
  }

  pKid->m_pElement =
      pdfium::MakeRetain<CPDF_StructElement>(m_pTree.Get(), this, pKidDict);
}
