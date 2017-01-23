// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssrulecollection.h"

#include <algorithm>
#include <map>
#include <memory>

#include "xfa/fde/css/cfde_cssdeclaration.h"
#include "xfa/fde/css/cfde_cssselector.h"
#include "xfa/fde/css/cfde_cssstylerule.h"
#include "xfa/fde/css/cfde_cssstylesheet.h"
#include "xfa/fde/css/cfde_csssyntaxparser.h"
#include "xfa/fde/css/cfde_csstagcache.h"

#define FDE_CSSUNIVERSALHASH ('*')

void CFDE_CSSRuleCollection::Clear() {
  m_IDRules.clear();
  m_TagRules.clear();
  m_ClassRules.clear();
  m_pUniversalRules = nullptr;
  m_iSelectors = 0;
}

CFDE_CSSRuleCollection::CFDE_CSSRuleCollection()
    : m_pUniversalRules(nullptr), m_iSelectors(0) {}

CFDE_CSSRuleCollection::~CFDE_CSSRuleCollection() {
  Clear();
}

void CFDE_CSSRuleCollection::AddRulesFrom(const CFDE_CSSStyleSheet* sheet,
                                          CFGAS_FontMgr* pFontMgr) {
  int32_t iRules = sheet->CountRules();
  for (int32_t j = 0; j < iRules; j++)
    AddRulesFrom(sheet, sheet->GetRule(j), pFontMgr);
}

void CFDE_CSSRuleCollection::AddRulesFrom(const CFDE_CSSStyleSheet* pStyleSheet,
                                          CFDE_CSSStyleRule* pStyleRule,
                                          CFGAS_FontMgr* pFontMgr) {
  CFDE_CSSDeclaration* pDeclaration = pStyleRule->GetDeclaration();
  int32_t iSelectors = pStyleRule->CountSelectorLists();
  for (int32_t i = 0; i < iSelectors; ++i) {
    CFDE_CSSSelector* pSelector = pStyleRule->GetSelectorList(i);
    if (pSelector->GetNameHash() != FDE_CSSUNIVERSALHASH) {
      AddRuleTo(&m_TagRules, pSelector->GetNameHash(), pSelector, pDeclaration);
      continue;
    }
    CFDE_CSSSelector* pNext = pSelector->GetNextSelector();
    if (!pNext) {
      Data* pData = NewRuleData(pSelector, pDeclaration);
      AddRuleTo(&m_pUniversalRules, pData);
      continue;
    }
    switch (pNext->GetType()) {
      case FDE_CSSSelectorType::ID:
        AddRuleTo(&m_IDRules, pNext->GetNameHash(), pSelector, pDeclaration);
        break;
      case FDE_CSSSelectorType::Class:
        AddRuleTo(&m_ClassRules, pNext->GetNameHash(), pSelector, pDeclaration);
        break;
      case FDE_CSSSelectorType::Descendant:
      case FDE_CSSSelectorType::Element:
        AddRuleTo(&m_pUniversalRules, NewRuleData(pSelector, pDeclaration));
        break;
      default:
        ASSERT(false);
        break;
    }
  }
}

void CFDE_CSSRuleCollection::AddRuleTo(std::map<uint32_t, Data*>* pMap,
                                       uint32_t dwKey,
                                       CFDE_CSSSelector* pSel,
                                       CFDE_CSSDeclaration* pDecl) {
  Data* pData = NewRuleData(pSel, pDecl);
  Data* pList = (*pMap)[dwKey];
  if (!pList) {
    (*pMap)[dwKey] = pData;
  } else if (AddRuleTo(&pList, pData)) {
    (*pMap)[dwKey] = pList;
  }
}

bool CFDE_CSSRuleCollection::AddRuleTo(Data** pList, Data* pData) {
  if (*pList) {
    pData->pNext = (*pList)->pNext;
    (*pList)->pNext = pData;
    return false;
  }
  *pList = pData;
  return true;
}

CFDE_CSSRuleCollection::Data* CFDE_CSSRuleCollection::NewRuleData(
    CFDE_CSSSelector* pSel,
    CFDE_CSSDeclaration* pDecl) {
  return new Data(pSel, pDecl, ++m_iSelectors);
}

CFDE_CSSRuleCollection::Data::Data(CFDE_CSSSelector* pSel,
                                   CFDE_CSSDeclaration* pDecl,
                                   uint32_t dwPos)
    : pSelector(pSel), pDeclaration(pDecl), dwPriority(dwPos), pNext(nullptr) {
  static const uint32_t s_Specific[5] = {0x00010000, 0x00010000, 0x00100000,
                                         0x00100000, 0x01000000};
  for (; pSel; pSel = pSel->GetNextSelector()) {
    FDE_CSSSelectorType eType = pSel->GetType();
    if (eType > FDE_CSSSelectorType::Descendant ||
        pSel->GetNameHash() != FDE_CSSUNIVERSALHASH) {
      dwPriority += s_Specific[static_cast<int>(eType)];
    }
  }
}
