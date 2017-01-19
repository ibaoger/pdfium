// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSRULECOLLECTION_H_
#define XFA_FDE_CSS_CFDE_CSSRULECOLLECTION_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/fx_basic.h"

class CFDE_CSSDeclaration;
class CFDE_CSSRule;
class CFDE_CSSSelector;
class CFDE_CSSStyleSheet;
class CFGAS_FontMgr;

class CFDE_CSSRuleCollection {
 public:
  class Data {
   public:
    Data(CFDE_CSSSelector* pSel, CFDE_CSSDeclaration* pDecl, uint32_t dwPos);

    CFDE_CSSSelector* const pSelector;
    CFDE_CSSDeclaration* const pDeclaration;
    uint32_t dwPriority;
  };

  CFDE_CSSRuleCollection();
  ~CFDE_CSSRuleCollection();

  void AddRulesFrom(const CFX_ArrayTemplate<CFDE_CSSStyleSheet*>& sheets,
                    uint32_t dwMediaList,
                    CFGAS_FontMgr* pFontMgr);
  void Clear();
  int32_t CountSelectors() const { return m_iSelectors; }

  const std::vector<std::unique_ptr<Data>>* GetTagRuleData(
      uint32_t dwTagHash) const {
    auto it = m_TagRules.find(dwTagHash);
    return it != m_TagRules.end() ? &it->second : nullptr;
  }

  const std::vector<std::unique_ptr<Data>>* GetClassRuleData(
      uint32_t dwIDHash) const {
    auto it = m_ClassRules.find(dwIDHash);
    return it != m_ClassRules.end() ? &it->second : nullptr;
  }

  const std::vector<std::unique_ptr<Data>>* GetUniversalRuleData() const {
    return &m_pUniversalRules;
  }
  const std::vector<std::unique_ptr<Data>>* GetPseudoRuleData() const {
    return &m_pPseudoRules;
  }

 protected:
  void AddRulesFrom(CFDE_CSSStyleSheet* pStyleSheet,
                    CFDE_CSSRule* pRule,
                    uint32_t dwMediaList,
                    CFGAS_FontMgr* pFontMgr);
  std::unique_ptr<Data> NewRuleData(CFDE_CSSSelector* pSel,
                                    CFDE_CSSDeclaration* pDecl);

  std::map<uint32_t, std::vector<std::unique_ptr<Data>>> m_TagRules;
  std::map<uint32_t, std::vector<std::unique_ptr<Data>>> m_ClassRules;
  std::vector<std::unique_ptr<Data>> m_pUniversalRules;
  std::vector<std::unique_ptr<Data>> m_pPseudoRules;
  int32_t m_iSelectors;
};

#endif  // XFA_FDE_CSS_CFDE_CSSRULECOLLECTION_H_
