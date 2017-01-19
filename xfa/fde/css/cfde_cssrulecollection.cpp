// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssrulecollection.h"

#include <algorithm>
#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fde/css/cfde_cssdeclaration.h"
#include "xfa/fde/css/cfde_cssmediarule.h"
#include "xfa/fde/css/cfde_cssrule.h"
#include "xfa/fde/css/cfde_cssselector.h"
#include "xfa/fde/css/cfde_cssstylerule.h"
#include "xfa/fde/css/cfde_cssstylesheet.h"
#include "xfa/fde/css/cfde_csssyntaxparser.h"
#include "xfa/fde/css/cfde_csstagcache.h"

#define FDE_CSSUNIVERSALHASH ('*')

void CFDE_CSSRuleCollection::Clear() {
  m_TagRules.clear();
  m_ClassRules.clear();
  m_pUniversalRules.clear();
  m_pPseudoRules.clear();
  m_iSelectors = 0;
}

CFDE_CSSRuleCollection::CFDE_CSSRuleCollection() : m_iSelectors(0) {}

CFDE_CSSRuleCollection::~CFDE_CSSRuleCollection() {
  Clear();
}

const std::vector<std::unique_ptr<CFDE_CSSRuleCollection::Data>>*
CFDE_CSSRuleCollection::GetTagRuleData(uint32_t dwTagHash) const {
  auto it = m_TagRules.find(dwTagHash);
  return it != m_TagRules.end() ? &it->second : nullptr;
}

const std::vector<std::unique_ptr<CFDE_CSSRuleCollection::Data>>*
CFDE_CSSRuleCollection::GetClassRuleData(uint32_t dwIDHash) const {
  auto it = m_ClassRules.find(dwIDHash);
  return it != m_ClassRules.end() ? &it->second : nullptr;
}

void CFDE_CSSRuleCollection::AddRulesFrom(
    const CFX_ArrayTemplate<CFDE_CSSStyleSheet*>& sheets,
    uint32_t dwMediaList,
    CFGAS_FontMgr* pFontMgr) {
  int32_t iSheets = sheets.GetSize();
  for (int32_t i = 0; i < iSheets; ++i) {
    CFDE_CSSStyleSheet* pSheet = sheets.GetAt(i);
    if (uint32_t dwMatchMedia = pSheet->GetMediaList() & dwMediaList) {
      int32_t iRules = pSheet->CountRules();
      for (int32_t j = 0; j < iRules; j++) {
        AddRulesFrom(pSheet, pSheet->GetRule(j), dwMatchMedia, pFontMgr);
      }
    }
  }
}

void CFDE_CSSRuleCollection::AddRulesFrom(CFDE_CSSStyleSheet* pStyleSheet,
                                          CFDE_CSSRule* pRule,
                                          uint32_t dwMediaList,
                                          CFGAS_FontMgr* pFontMgr) {
  switch (pRule->GetType()) {
    case FDE_CSSRuleType::Style: {
      CFDE_CSSStyleRule* pStyleRule = static_cast<CFDE_CSSStyleRule*>(pRule);
      CFDE_CSSDeclaration* pDeclaration = pStyleRule->GetDeclaration();
      int32_t iSelectors = pStyleRule->CountSelectorLists();
      for (int32_t i = 0; i < iSelectors; ++i) {
        CFDE_CSSSelector* pSelector = pStyleRule->GetSelectorList(i);
        if (pSelector->GetType() == FDE_CSSSelectorType::Pseudo) {
          m_pPseudoRules.push_back(NewRuleData(pSelector, pDeclaration));
          continue;
        }
        if (pSelector->GetNameHash() != FDE_CSSUNIVERSALHASH) {
          m_TagRules[pSelector->GetNameHash()].push_back(
              NewRuleData(pSelector, pDeclaration));
          continue;
        }

        CFDE_CSSSelector* pNext = pSelector->GetNextSelector();
        if (!pNext) {
          m_pUniversalRules.push_back(NewRuleData(pSelector, pDeclaration));
          continue;
        }

        switch (pNext->GetType()) {
          case FDE_CSSSelectorType::ID:
            break;
          case FDE_CSSSelectorType::Class:
            m_ClassRules[pNext->GetNameHash()].push_back(
                NewRuleData(pSelector, pDeclaration));
            break;
          case FDE_CSSSelectorType::Descendant:
          case FDE_CSSSelectorType::Element:
            m_pUniversalRules.push_back(NewRuleData(pSelector, pDeclaration));
            break;
          default:
            ASSERT(false);
            break;
        }
      }
    } break;
    case FDE_CSSRuleType::Media: {
      CFDE_CSSMediaRule* pMediaRule = static_cast<CFDE_CSSMediaRule*>(pRule);
      if (pMediaRule->GetMediaList() & dwMediaList) {
        int32_t iRules = pMediaRule->CountRules();
        for (int32_t i = 0; i < iRules; ++i) {
          AddRulesFrom(pStyleSheet, pMediaRule->GetRule(i), dwMediaList,
                       pFontMgr);
        }
      }
    } break;
    default:
      break;
  }
}

std::unique_ptr<CFDE_CSSRuleCollection::Data>
CFDE_CSSRuleCollection::NewRuleData(CFDE_CSSSelector* pSel,
                                    CFDE_CSSDeclaration* pDecl) {
  return pdfium::MakeUnique<Data>(pSel, pDecl, ++m_iSelectors);
}

CFDE_CSSRuleCollection::Data::Data(CFDE_CSSSelector* pSel,
                                   CFDE_CSSDeclaration* pDecl,
                                   uint32_t dwPos)
    : pSelector(pSel), pDeclaration(pDecl), dwPriority(dwPos) {
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
