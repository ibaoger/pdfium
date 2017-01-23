// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssstylesheet.h"

#include <utility>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/css/cfde_cssdeclaration.h"
#include "xfa/fde/css/cfde_cssstylerule.h"
#include "xfa/fde/css/fde_cssdatatable.h"
#include "xfa/fgas/crt/fgas_codepage.h"

CFDE_CSSStyleSheet::CFDE_CSSStyleSheet() {}

CFDE_CSSStyleSheet::~CFDE_CSSStyleSheet() {
  Reset();
}

void CFDE_CSSStyleSheet::Reset() {
  m_RuleArray.clear();
  m_StringCache.clear();
}

int32_t CFDE_CSSStyleSheet::CountRules() const {
  return pdfium::CollectionSize<int32_t>(m_RuleArray);
}

CFDE_CSSStyleRule* CFDE_CSSStyleSheet::GetRule(int32_t index) const {
  return m_RuleArray[index].get();
}

bool CFDE_CSSStyleSheet::LoadBuffer(const FX_WCHAR* pBuffer, int32_t iBufSize) {
  ASSERT(pBuffer && iBufSize > 0);

  auto pSyntax = pdfium::MakeUnique<CFDE_CSSSyntaxParser>();
  if (!pSyntax->Init(pBuffer, iBufSize))
    return false;

  Reset();
  FDE_CSSSyntaxStatus eStatus;
  do {
    switch (eStatus = pSyntax->DoSyntaxParse()) {
      case FDE_CSSSyntaxStatus::StyleRule:
        eStatus = LoadStyleRule(pSyntax.get(), &m_RuleArray);
        break;
      default:
        break;
    }
  } while (eStatus >= FDE_CSSSyntaxStatus::None);

  m_StringCache.clear();
  return eStatus != FDE_CSSSyntaxStatus::Error;
}

FDE_CSSSyntaxStatus CFDE_CSSStyleSheet::LoadStyleRule(
    CFDE_CSSSyntaxParser* pSyntax,
    std::vector<std::unique_ptr<CFDE_CSSStyleRule>>* ruleArray) {
  std::vector<std::unique_ptr<CFDE_CSSSelector>> selectors;

  CFDE_CSSStyleRule* pStyleRule = nullptr;
  const FX_WCHAR* pszValue = nullptr;
  int32_t iValueLen = 0;
  FDE_CSSPropertyArgs propertyArgs;
  propertyArgs.pStringCache = &m_StringCache;
  propertyArgs.pProperty = nullptr;
  CFX_WideString wsName;
  while (1) {
    switch (pSyntax->DoSyntaxParse()) {
      case FDE_CSSSyntaxStatus::Selector: {
        pszValue = pSyntax->GetCurrentString(iValueLen);
        auto pSelector = CFDE_CSSSelector::FromString(pszValue, iValueLen);
        if (pSelector)
          selectors.push_back(std::move(pSelector));
        break;
      }
      case FDE_CSSSyntaxStatus::PropertyName:
        pszValue = pSyntax->GetCurrentString(iValueLen);
        propertyArgs.pProperty =
            FDE_GetCSSPropertyByName(CFX_WideStringC(pszValue, iValueLen));
        if (!propertyArgs.pProperty)
          wsName = CFX_WideStringC(pszValue, iValueLen);
        break;
      case FDE_CSSSyntaxStatus::PropertyValue:
        if (propertyArgs.pProperty) {
          pszValue = pSyntax->GetCurrentString(iValueLen);
          if (iValueLen > 0) {
            pStyleRule->GetDeclaration()->AddProperty(&propertyArgs, pszValue,
                                                      iValueLen);
          }
        } else if (iValueLen > 0) {
          pszValue = pSyntax->GetCurrentString(iValueLen);
          if (iValueLen > 0) {
            pStyleRule->GetDeclaration()->AddProperty(
                &propertyArgs, wsName.c_str(), wsName.GetLength(), pszValue,
                iValueLen);
          }
        }
        break;
      case FDE_CSSSyntaxStatus::DeclOpen:
        if (!pStyleRule && !selectors.empty()) {
          auto rule = pdfium::MakeUnique<CFDE_CSSStyleRule>();
          pStyleRule = rule.get();
          pStyleRule->SetSelector(&selectors);
          ruleArray->push_back(std::move(rule));
        } else {
          SkipRuleSet(pSyntax);
          return FDE_CSSSyntaxStatus::None;
        }
        break;
      case FDE_CSSSyntaxStatus::DeclClose:
        if (pStyleRule && pStyleRule->GetDeclaration()->empty()) {
          ruleArray->pop_back();
          pStyleRule = nullptr;
        }
        return FDE_CSSSyntaxStatus::None;
      case FDE_CSSSyntaxStatus::EOS:
        return FDE_CSSSyntaxStatus::EOS;
      case FDE_CSSSyntaxStatus::Error:
      default:
        return FDE_CSSSyntaxStatus::Error;
    }
  }
}

void CFDE_CSSStyleSheet::SkipRuleSet(CFDE_CSSSyntaxParser* pSyntax) {
  while (1) {
    switch (pSyntax->DoSyntaxParse()) {
      case FDE_CSSSyntaxStatus::Selector:
      case FDE_CSSSyntaxStatus::DeclOpen:
      case FDE_CSSSyntaxStatus::PropertyName:
      case FDE_CSSSyntaxStatus::PropertyValue:
        break;
      case FDE_CSSSyntaxStatus::DeclClose:
      case FDE_CSSSyntaxStatus::EOS:
      case FDE_CSSSyntaxStatus::Error:
      default:
        return;
    }
  }
}
