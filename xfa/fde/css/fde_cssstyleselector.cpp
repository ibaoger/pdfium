// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/fde_cssstyleselector.h"

#include <algorithm>
#include <memory>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/css/fde_csscache.h"
#include "xfa/fde/css/fde_cssdeclaration.h"
#include "xfa/fde/css/fde_cssstylesheet.h"
#include "xfa/fde/css/fde_csssyntax.h"
#include "xfa/fxfa/app/cxfa_csstagprovider.h"

#define FDE_CSSUNIVERSALHASH ('*')

int32_t CFDE_CSSCounterStyle::FindIndex(const FX_WCHAR* pszIdentifier) {
  int32_t iCount = m_arrCounterData.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    if (FXSYS_wcscmp(pszIdentifier, m_arrCounterData.ElementAt(i).m_pszIdent) ==
        0) {
      return i;
    }
  }
  return -1;
}

void CFDE_CSSCounterStyle::DoUpdateIndex(IFDE_CSSValueList* pList) {
  if (!pList)
    return;

  int32_t iCount = pList->CountValues();
  FX_FLOAT fDefValue = 1.0;
  bool bDefIncrement = true;
  if (pList == m_pCounterReset) {
    fDefValue = 0.0;
    bDefIncrement = false;
  }
  for (int32_t i = 0; i < iCount; i++) {
    IFDE_CSSValueList* pCounter =
        static_cast<IFDE_CSSValueList*>(pList->GetValue(i));
    int32_t iLen;
    const FX_WCHAR* pszIdentifier =
        static_cast<IFDE_CSSPrimitiveValue*>(pCounter->GetValue(0))
            ->GetString(iLen);
    FX_FLOAT fValue = fDefValue;
    if (pCounter->CountValues() > 1) {
      fValue = static_cast<IFDE_CSSPrimitiveValue*>(pCounter->GetValue(1))
                   ->GetFloat();
    }
    int32_t iIndex = FindIndex(pszIdentifier);
    if (iIndex == -1) {
      FDE_CSSCOUNTERDATA data;
      data.m_pszIdent = pszIdentifier;
      if (bDefIncrement) {
        data.m_bIncrement = true;
        data.m_iIncVal = (int32_t)fValue;
      } else {
        data.m_iResetVal = (int32_t)fValue;
        data.m_bReset = true;
      }
      m_arrCounterData.Add(data);
    } else {
      FDE_CSSCOUNTERDATA& data = m_arrCounterData.ElementAt(iIndex);
      if (bDefIncrement) {
        data.m_bIncrement = true;
        data.m_iIncVal += (int32_t)fValue;
      } else {
        data.m_bReset = true;
        data.m_iResetVal = (int32_t)fValue;
      }
    }
  }
}

CFDE_CSSCounterStyle::CFDE_CSSCounterStyle()
    : m_pCounterInc(nullptr), m_pCounterReset(nullptr), m_bIndexDirty(false) {}

CFDE_CSSCounterStyle::~CFDE_CSSCounterStyle() {}

void CFDE_CSSCounterStyle::UpdateIndex() {
  if (!m_bIndexDirty) {
    return;
  }
  m_arrCounterData.RemoveAll();
  DoUpdateIndex(m_pCounterInc);
  DoUpdateIndex(m_pCounterReset);
  m_bIndexDirty = false;
}

FDE_CSSRuleData::FDE_CSSRuleData(CFDE_CSSSelector* pSel,
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

CFDE_CSSStyleSelector::CFDE_CSSStyleSelector(CFGAS_FontMgr* pFontMgr)
    : m_pFontMgr(pFontMgr), m_fDefFontSize(12.0f) {
  m_ePriorities[static_cast<int32_t>(FDE_CSSStyleSheetPriority::High)] =
      FDE_CSSStyleSheetGroup::Author;
  m_ePriorities[static_cast<int32_t>(FDE_CSSStyleSheetPriority::Mid)] =
      FDE_CSSStyleSheetGroup::User;
  m_ePriorities[static_cast<int32_t>(FDE_CSSStyleSheetPriority::Low)] =
      FDE_CSSStyleSheetGroup::UserAgent;
}

CFDE_CSSStyleSelector::~CFDE_CSSStyleSelector() {
  Reset();
}

void CFDE_CSSStyleSelector::SetDefFontSize(FX_FLOAT fFontSize) {
  ASSERT(fFontSize > 0);
  m_fDefFontSize = fFontSize;
}

CFDE_CSSAccelerator* CFDE_CSSStyleSelector::InitAccelerator() {
  if (!m_pAccelerator)
    m_pAccelerator = pdfium::MakeUnique<CFDE_CSSAccelerator>();
  m_pAccelerator->Clear();
  return m_pAccelerator.get();
}

IFDE_CSSComputedStyle* CFDE_CSSStyleSelector::CreateComputedStyle(
    IFDE_CSSComputedStyle* pParentStyle) {
  CFDE_CSSComputedStyle* pStyle = new CFDE_CSSComputedStyle();
  if (pParentStyle) {
    pStyle->m_InheritedData =
        static_cast<CFDE_CSSComputedStyle*>(pParentStyle)->m_InheritedData;
  } else {
    pStyle->m_InheritedData.Reset();
  }
  pStyle->m_NonInheritedData.Reset();
  return pStyle;
}

bool CFDE_CSSStyleSelector::SetStyleSheet(FDE_CSSStyleSheetGroup eType,
                                          IFDE_CSSStyleSheet* pSheet) {
  CFX_ArrayTemplate<IFDE_CSSStyleSheet*>& dest =
      m_SheetGroups[static_cast<int32_t>(eType)];
  dest.RemoveAt(0, dest.GetSize());
  if (pSheet)
    dest.Add(pSheet);
  return true;
}

bool CFDE_CSSStyleSelector::SetStyleSheets(
    FDE_CSSStyleSheetGroup eType,
    const CFX_ArrayTemplate<IFDE_CSSStyleSheet*>* pArray) {
  CFX_ArrayTemplate<IFDE_CSSStyleSheet*>& dest =
      m_SheetGroups[static_cast<int32_t>(eType)];
  if (pArray)
    dest.Copy(*pArray);
  else
    dest.RemoveAt(0, dest.GetSize());
  return true;
}

void CFDE_CSSStyleSelector::SetStylePriority(
    FDE_CSSStyleSheetGroup eType,
    FDE_CSSStyleSheetPriority ePriority) {
  m_ePriorities[static_cast<int32_t>(ePriority)] = eType;
}

void CFDE_CSSStyleSelector::UpdateStyleIndex(uint32_t dwMediaList) {
  Reset();

  // TODO(dsinclair): Hard coded size bad. This should probably just be a map.
  for (int32_t iGroup = 0; iGroup < 3; ++iGroup) {
    CFDE_CSSRuleCollection& rules = m_RuleCollection[iGroup];
    rules.AddRulesFrom(m_SheetGroups[iGroup], dwMediaList, m_pFontMgr);
  }
}

void CFDE_CSSStyleSelector::Reset() {
  // TODO(dsinclair): Hard coded size bad. This should probably just be a map.
  for (int32_t iGroup = 0; iGroup < 3; ++iGroup) {
    m_RuleCollection[iGroup].Clear();
  }
}

int32_t CFDE_CSSStyleSelector::MatchDeclarations(
    CXFA_CSSTagProvider* pTag,
    CFDE_CSSDeclarationArray& matchedDecls,
    FDE_CSSPseudo ePseudoType) {
  ASSERT(pTag);
  FDE_CSSTagCache* pCache = m_pAccelerator->GetTopElement();
  ASSERT(pCache && pCache->GetTag() == pTag);

  matchedDecls.RemoveAt(0, matchedDecls.GetSize());
  // TODO(dsinclair): Hard coded size bad ...
  for (int32_t ePriority = 2; ePriority >= 0; --ePriority) {
    FDE_CSSStyleSheetGroup eGroup = m_ePriorities[ePriority];
    CFDE_CSSRuleCollection& rules =
        m_RuleCollection[static_cast<int32_t>(eGroup)];
    if (rules.CountSelectors() == 0)
      continue;

    if (ePseudoType == FDE_CSSPseudo::NONE) {
      MatchRules(pCache, rules.GetUniversalRuleData(), ePseudoType);
      if (pCache->HashTag()) {
        MatchRules(pCache, rules.GetTagRuleData(pCache->HashTag()),
                   ePseudoType);
      }
      int32_t iCount = pCache->CountHashClass();
      for (int32_t i = 0; i < iCount; i++) {
        pCache->SetClassIndex(i);
        MatchRules(pCache, rules.GetClassRuleData(pCache->HashClass()),
                   ePseudoType);
      }
    } else {
      MatchRules(pCache, rules.GetPseudoRuleData(), ePseudoType);
    }

    std::sort(m_MatchedRules.begin(), m_MatchedRules.end(),
              [](const FDE_CSSRuleData* p1, const FDE_CSSRuleData* p2) {
                return p1->dwPriority < p2->dwPriority;
              });
    for (const auto& rule : m_MatchedRules)
      matchedDecls.Add(rule->pDeclaration);
    m_MatchedRules.clear();
  }
  return matchedDecls.GetSize();
}

void CFDE_CSSStyleSelector::MatchRules(FDE_CSSTagCache* pCache,
                                       FDE_CSSRuleData* pList,
                                       FDE_CSSPseudo ePseudoType) {
  while (pList) {
    if (MatchSelector(pCache, pList->pSelector, ePseudoType))
      m_MatchedRules.push_back(pList);
    pList = pList->pNext;
  }
}

bool CFDE_CSSStyleSelector::MatchSelector(FDE_CSSTagCache* pCache,
                                          CFDE_CSSSelector* pSel,
                                          FDE_CSSPseudo ePseudoType) {
  uint32_t dwHash;
  while (pSel && pCache) {
    switch (pSel->GetType()) {
      case FDE_CSSSelectorType::Descendant:
        dwHash = pSel->GetNameHash();
        while ((pCache = pCache->GetParent()) != nullptr) {
          if (dwHash != FDE_CSSUNIVERSALHASH && dwHash != pCache->HashTag()) {
            continue;
          }
          if (MatchSelector(pCache, pSel->GetNextSelector(), ePseudoType)) {
            return true;
          }
        }
        return false;
      case FDE_CSSSelectorType::ID:
        dwHash = pCache->HashID();
        if (dwHash != pSel->GetNameHash()) {
          return false;
        }
        break;
      case FDE_CSSSelectorType::Class:
        dwHash = pCache->HashClass();
        if (dwHash != pSel->GetNameHash()) {
          return false;
        }
        break;
      case FDE_CSSSelectorType::Element:
        dwHash = pSel->GetNameHash();
        if (dwHash != FDE_CSSUNIVERSALHASH && dwHash != pCache->HashTag()) {
          return false;
        }
        break;
      case FDE_CSSSelectorType::Pseudo:
        dwHash = FDE_GetCSSPseudoByEnum(ePseudoType)->dwHash;
        if (dwHash != pSel->GetNameHash()) {
          return false;
        }
        break;
      default:
        ASSERT(false);
        break;
    }
    pSel = pSel->GetNextSelector();
  }
  return !pSel && pCache;
}

void CFDE_CSSStyleSelector::ComputeStyle(
    CXFA_CSSTagProvider* pTag,
    const CFDE_CSSDeclaration** ppDeclArray,
    int32_t iDeclCount,
    IFDE_CSSComputedStyle* pDestStyle) {
  ASSERT(iDeclCount >= 0);
  ASSERT(pDestStyle);

  static const uint32_t s_dwStyleHash = FX_HashCode_GetW(L"style", true);
  static const uint32_t s_dwAlignHash = FX_HashCode_GetW(L"align", true);

  if (!pTag->empty()) {
    CFDE_CSSDeclaration* pDecl = nullptr;
    for (auto it : *pTag) {
      CFX_WideString wsAttri = it.first;
      CFX_WideString wsValue = it.second;
      uint32_t dwAttriHash = FX_HashCode_GetW(wsAttri.AsStringC(), true);
      if (dwAttriHash == s_dwStyleHash) {
        if (!pDecl)
          pDecl = new CFDE_CSSDeclaration;

        AppendInlineStyle(pDecl, wsValue.c_str(), wsValue.GetLength());
      } else if (dwAttriHash == s_dwAlignHash) {
        if (!pDecl)
          pDecl = new CFDE_CSSDeclaration;

        FDE_CSSPropertyARGS args;
        args.pStringCache = nullptr;
        args.pProperty = FDE_GetCSSPropertyByEnum(FDE_CSSProperty::TextAlign);
        pDecl->AddProperty(&args, wsValue.c_str(), wsValue.GetLength());
      }
    }

    if (pDecl) {
      CFDE_CSSDeclarationArray decls;
      decls.SetSize(iDeclCount + 1);
      CFDE_CSSDeclaration** ppInline = decls.GetData();
      FXSYS_memcpy(ppInline, ppDeclArray,
                   iDeclCount * sizeof(CFDE_CSSDeclaration*));
      ppInline[iDeclCount++] = pDecl;
      ApplyDeclarations(true, const_cast<const CFDE_CSSDeclaration**>(ppInline),
                        iDeclCount, pDestStyle);
      ApplyDeclarations(false,
                        const_cast<const CFDE_CSSDeclaration**>(ppInline),
                        iDeclCount, pDestStyle);
      return;
    }
  }

  if (iDeclCount > 0) {
    ASSERT(ppDeclArray);

    ApplyDeclarations(true, ppDeclArray, iDeclCount, pDestStyle);
    ApplyDeclarations(false, ppDeclArray, iDeclCount, pDestStyle);
  }
}

void CFDE_CSSStyleSelector::ApplyDeclarations(
    bool bPriority,
    const CFDE_CSSDeclaration** ppDeclArray,
    int32_t iDeclCount,
    IFDE_CSSComputedStyle* pDestStyle) {
  CFDE_CSSComputedStyle* pComputedStyle =
      static_cast<CFDE_CSSComputedStyle*>(pDestStyle);
  IFDE_CSSValue* pVal;
  bool bImportant;
  int32_t i;
  if (bPriority) {
    IFDE_CSSValue* pLastest = nullptr;
    IFDE_CSSValue* pImportant = nullptr;
    for (i = 0; i < iDeclCount; ++i) {
      pVal = ppDeclArray[i]->GetProperty(FDE_CSSProperty::FontSize, bImportant);
      if (!pVal)
        continue;

      if (bImportant)
        pImportant = pVal;
      else
        pLastest = pVal;
    }
    if (pImportant) {
      ApplyProperty(FDE_CSSProperty::FontSize, pImportant, pComputedStyle);
    } else if (pLastest) {
      ApplyProperty(FDE_CSSProperty::FontSize, pLastest, pComputedStyle);
    }
  } else {
    CFDE_CSSDeclarationArray importants;
    const CFDE_CSSDeclaration* pDecl = nullptr;
    FDE_CSSProperty eProp;
    FX_POSITION pos;
    for (i = 0; i < iDeclCount; ++i) {
      pDecl = ppDeclArray[i];
      pos = pDecl->GetStartPosition();
      while (pos) {
        pDecl->GetNextProperty(pos, eProp, pVal, bImportant);
        if (eProp == FDE_CSSProperty::FontSize) {
          continue;
        } else if (!bImportant) {
          ApplyProperty(eProp, pVal, pComputedStyle);
        } else if (importants.GetSize() == 0 ||
                   importants[importants.GetUpperBound()] != pDecl) {
          importants.Add(const_cast<CFDE_CSSDeclaration*>(pDecl));
        }
      }
    }
    iDeclCount = importants.GetSize();
    for (i = 0; i < iDeclCount; ++i) {
      pDecl = importants[i];
      pos = pDecl->GetStartPosition();
      while (pos) {
        pDecl->GetNextProperty(pos, eProp, pVal, bImportant);
        if (bImportant && eProp != FDE_CSSProperty::FontSize) {
          ApplyProperty(eProp, pVal, pComputedStyle);
        }
      }
    }
    CFX_WideString wsName, wsValue;
    pos = pDecl->GetStartCustom();
    while (pos) {
      pDecl->GetNextCustom(pos, wsName, wsValue);
      pComputedStyle->AddCustomStyle(wsName, wsValue);
    }
  }
}

void CFDE_CSSStyleSelector::AppendInlineStyle(CFDE_CSSDeclaration* pDecl,
                                              const FX_WCHAR* psz,
                                              int32_t iLen) {
  ASSERT(pDecl && psz && iLen > 0);
  std::unique_ptr<CFDE_CSSSyntaxParser> pSyntax(new CFDE_CSSSyntaxParser);
  if (!pSyntax->Init(psz, iLen, 32, true))
    return;

  int32_t iLen2 = 0;
  const FX_WCHAR* psz2;
  FDE_CSSPropertyARGS args;
  args.pStringCache = nullptr;
  args.pProperty = nullptr;
  CFX_WideString wsName;
  while (1) {
    FDE_CSSSyntaxStatus eStatus = pSyntax->DoSyntaxParse();
    if (eStatus == FDE_CSSSyntaxStatus::PropertyName) {
      psz2 = pSyntax->GetCurrentString(iLen2);
      args.pProperty = FDE_GetCSSPropertyByName(CFX_WideStringC(psz2, iLen2));
      if (!args.pProperty)
        wsName = CFX_WideStringC(psz2, iLen2);
    } else if (eStatus == FDE_CSSSyntaxStatus::PropertyValue) {
      if (args.pProperty) {
        psz2 = pSyntax->GetCurrentString(iLen2);
        if (iLen2 > 0)
          pDecl->AddProperty(&args, psz2, iLen2);
      } else if (iLen2 > 0) {
        psz2 = pSyntax->GetCurrentString(iLen2);
        if (iLen2 > 0) {
          pDecl->AddProperty(&args, wsName.c_str(), wsName.GetLength(), psz2,
                             iLen2);
        }
      }
    } else {
      break;
    }
  }
}

#define FDE_CSSNONINHERITS (pComputedStyle->m_NonInheritedData)
#define FDE_CSSINHERITS (pComputedStyle->m_InheritedData)
#define FDE_CSSFONTSIZE (FDE_CSSINHERITS.m_fFontSize)
void CFDE_CSSStyleSelector::ApplyProperty(
    FDE_CSSProperty eProperty,
    IFDE_CSSValue* pValue,
    CFDE_CSSComputedStyle* pComputedStyle) {
  if (pValue->GetType() == FDE_CSSVALUETYPE_Primitive) {
    IFDE_CSSPrimitiveValue* pPrimitive =
        static_cast<IFDE_CSSPrimitiveValue*>(pValue);
    FDE_CSSPrimitiveType eType = pPrimitive->GetPrimitiveType();
    switch (eProperty) {
      case FDE_CSSProperty::Display:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_eDisplay = ToDisplay(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::FontSize: {
        FX_FLOAT& fFontSize = FDE_CSSFONTSIZE;
        if (eType >= FDE_CSSPrimitiveType::Number &&
            eType <= FDE_CSSPrimitiveType::Picas) {
          fFontSize = ApplyNumber(eType, pPrimitive->GetFloat(), fFontSize);
        } else if (eType == FDE_CSSPrimitiveType::Enum) {
          fFontSize = ToFontSize(pPrimitive->GetEnum(), fFontSize);
        }
      } break;
      case FDE_CSSProperty::LineHeight:
        if (eType == FDE_CSSPrimitiveType::Number) {
          FDE_CSSINHERITS.m_fLineHeight =
              pPrimitive->GetFloat() * FDE_CSSFONTSIZE;
        } else if (eType > FDE_CSSPrimitiveType::Number &&
                   eType <= FDE_CSSPrimitiveType::Picas) {
          FDE_CSSINHERITS.m_fLineHeight =
              ApplyNumber(eType, pPrimitive->GetFloat(), FDE_CSSFONTSIZE);
        }
        break;
      case FDE_CSSProperty::TextAlign:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_eTextAlign = ToTextAlign(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::TextIndent:
        SetLengthWithPercent(FDE_CSSINHERITS.m_TextIndent, eType, pPrimitive,
                             FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::FontWeight:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_wFontWeight = ToFontWeight(pPrimitive->GetEnum());
        } else if (eType == FDE_CSSPrimitiveType::Number) {
          int32_t iValue = (int32_t)pPrimitive->GetFloat() / 100;
          if (iValue >= 1 && iValue <= 9) {
            FDE_CSSINHERITS.m_wFontWeight = iValue * 100;
          }
        }
        break;
      case FDE_CSSProperty::FontStyle:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_eFontStyle = ToFontStyle(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::Color:
        if (eType == FDE_CSSPrimitiveType::RGB) {
          FDE_CSSINHERITS.m_dwFontColor = pPrimitive->GetRGBColor();
        }
        break;
      case FDE_CSSProperty::MarginLeft:
        FDE_CSSNONINHERITS.m_bHasMargin |=
            SetLengthWithPercent(FDE_CSSNONINHERITS.m_MarginWidth.left, eType,
                                 pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::MarginTop:
        FDE_CSSNONINHERITS.m_bHasMargin |=
            SetLengthWithPercent(FDE_CSSNONINHERITS.m_MarginWidth.top, eType,
                                 pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::MarginRight:
        FDE_CSSNONINHERITS.m_bHasMargin |=
            SetLengthWithPercent(FDE_CSSNONINHERITS.m_MarginWidth.right, eType,
                                 pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::MarginBottom:
        FDE_CSSNONINHERITS.m_bHasMargin |=
            SetLengthWithPercent(FDE_CSSNONINHERITS.m_MarginWidth.bottom, eType,
                                 pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::PaddingLeft:
        FDE_CSSNONINHERITS.m_bHasPadding |=
            SetLengthWithPercent(FDE_CSSNONINHERITS.m_PaddingWidth.left, eType,
                                 pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::PaddingTop:
        FDE_CSSNONINHERITS.m_bHasPadding |=
            SetLengthWithPercent(FDE_CSSNONINHERITS.m_PaddingWidth.top, eType,
                                 pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::PaddingRight:
        FDE_CSSNONINHERITS.m_bHasPadding |=
            SetLengthWithPercent(FDE_CSSNONINHERITS.m_PaddingWidth.right, eType,
                                 pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::PaddingBottom:
        FDE_CSSNONINHERITS.m_bHasPadding |=
            SetLengthWithPercent(FDE_CSSNONINHERITS.m_PaddingWidth.bottom,
                                 eType, pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::BorderLeftWidth:
        FDE_CSSNONINHERITS.m_bHasBorder |=
            SetLengthWithPercent(FDE_CSSNONINHERITS.m_BorderWidth.left, eType,
                                 pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::BorderTopWidth:
        FDE_CSSNONINHERITS.m_bHasBorder |=
            SetLengthWithPercent(FDE_CSSNONINHERITS.m_BorderWidth.top, eType,
                                 pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::BorderRightWidth:
        FDE_CSSNONINHERITS.m_bHasBorder |=
            SetLengthWithPercent(FDE_CSSNONINHERITS.m_BorderWidth.right, eType,
                                 pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::BorderBottomWidth:
        FDE_CSSNONINHERITS.m_bHasBorder |=
            SetLengthWithPercent(FDE_CSSNONINHERITS.m_BorderWidth.bottom, eType,
                                 pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::BorderLeftStyle:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_eBDRLeftStyle =
              ToBorderStyle(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::BorderTopStyle:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_eBDRTopStyle =
              ToBorderStyle(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::BorderRightStyle:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_eBDRRightStyle =
              ToBorderStyle(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::BorderBottomStyle:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_eBDRBottomStyle =
              ToBorderStyle(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::BorderLeftColor:
        if (eType == FDE_CSSPrimitiveType::RGB) {
          FDE_CSSNONINHERITS.m_dwBDRLeftColor = pPrimitive->GetRGBColor();
        }
        break;
      case FDE_CSSProperty::BorderTopColor:
        if (eType == FDE_CSSPrimitiveType::RGB) {
          FDE_CSSNONINHERITS.m_dwBDRTopColor = pPrimitive->GetRGBColor();
        }
        break;
      case FDE_CSSProperty::BorderRightColor:
        if (eType == FDE_CSSPrimitiveType::RGB) {
          FDE_CSSNONINHERITS.m_dwBDRRightColor = pPrimitive->GetRGBColor();
        }
        break;
      case FDE_CSSProperty::BorderBottomColor:
        if (eType == FDE_CSSPrimitiveType::RGB) {
          FDE_CSSNONINHERITS.m_dwBDRBottomColor = pPrimitive->GetRGBColor();
        }
        break;
      case FDE_CSSProperty::ListStyleType:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_eListStyleType =
              ToListStyleType(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::ListStylePosition:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_eListStylePosition =
              ToListStylePosition(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::BackgroundColor:
        if (eType == FDE_CSSPrimitiveType::RGB) {
          FDE_CSSNONINHERITS.m_dwBKGColor = pPrimitive->GetRGBColor();
        } else if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_dwBKGColor = 0;
        }
        break;
      case FDE_CSSProperty::Visibility:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_eVisibility = ToVisibility(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::Width:
        SetLengthWithPercent(FDE_CSSNONINHERITS.m_BoxSize.cx, eType, pPrimitive,
                             FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::Height:
        SetLengthWithPercent(FDE_CSSNONINHERITS.m_BoxSize.cy, eType, pPrimitive,
                             FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::MinWidth:
        SetLengthWithPercent(FDE_CSSNONINHERITS.m_MinBoxSize.cx, eType,
                             pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::MinHeight:
        SetLengthWithPercent(FDE_CSSNONINHERITS.m_MinBoxSize.cy, eType,
                             pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::MaxWidth:
        SetLengthWithPercent(FDE_CSSNONINHERITS.m_MaxBoxSize.cx, eType,
                             pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::MaxHeight:
        SetLengthWithPercent(FDE_CSSNONINHERITS.m_MaxBoxSize.cy, eType,
                             pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::VerticalAlign:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_eVerticalAlign =
              ToVerticalAlign(pPrimitive->GetEnum());
        } else if (eType >= FDE_CSSPrimitiveType::Number &&
                   eType <= FDE_CSSPrimitiveType::Picas) {
          FDE_CSSNONINHERITS.m_eVerticalAlign = FDE_CSSVerticalAlign::Number;
          FDE_CSSNONINHERITS.m_fVerticalAlign =
              ApplyNumber(eType, pPrimitive->GetFloat(), FDE_CSSFONTSIZE);
        }
        break;
      case FDE_CSSProperty::WhiteSpace:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_eWhiteSpace = ToWhiteSpace(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::TextTransform:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_eTextTransform =
              ToTextTransform(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::FontVariant:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_eFontVariant = ToFontVariant(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::LetterSpacing:
        if (eType == FDE_CSSPrimitiveType::Percent) {
          break;
        } else if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_LetterSpacing.Set(FDE_CSSLengthUnit::Normal);
        } else if (eType >= FDE_CSSPrimitiveType::Number &&
                   eType <= FDE_CSSPrimitiveType::Picas) {
          SetLengthWithPercent(FDE_CSSINHERITS.m_LetterSpacing, eType,
                               pPrimitive, FDE_CSSFONTSIZE);
        }
        break;
      case FDE_CSSProperty::WordSpacing:
        if (eType == FDE_CSSPrimitiveType::Percent) {
          break;
        } else if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_WordSpacing.Set(FDE_CSSLengthUnit::Normal);
        } else if (eType >= FDE_CSSPrimitiveType::Number &&
                   eType <= FDE_CSSPrimitiveType::Picas) {
          SetLengthWithPercent(FDE_CSSINHERITS.m_WordSpacing, eType, pPrimitive,
                               FDE_CSSFONTSIZE);
        }
        break;
      case FDE_CSSProperty::Float:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_eFloat = ToFloat(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::Clear:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_eClear = ToClear(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::WritingMode:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_eWritingMode = ToWritingMode(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::WordBreak:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_eWordBreak = ToWordBreak(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::Widows:
        if (eType == FDE_CSSPrimitiveType::Number) {
          FDE_CSSINHERITS.m_iWidows = (int32_t)pPrimitive->GetFloat();
        }
        break;
      case FDE_CSSProperty::Orphans:
        if (eType == FDE_CSSPrimitiveType::Number) {
          FDE_CSSINHERITS.m_iOrphans = (int32_t)pPrimitive->GetFloat();
        }
        break;
      case FDE_CSSProperty::TextEmphasisColor:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          switch (pPrimitive->GetEnum()) {
            case FDE_CSSPropertyValue::Transparent:
              FDE_CSSINHERITS.m_dwTextEmphasisColor = 0;
              FDE_CSSINHERITS.m_bTextEmphasisColorCurrent = false;
              break;
            case FDE_CSSPropertyValue::Currentcolor:
              FDE_CSSINHERITS.m_bTextEmphasisColorCurrent = true;
              break;
            default:
              break;
          }
        } else if (eType == FDE_CSSPrimitiveType::RGB) {
          FDE_CSSINHERITS.m_dwTextEmphasisColor = pPrimitive->GetRGBColor();
          FDE_CSSINHERITS.m_bTextEmphasisColorCurrent = false;
        }
        break;
      case FDE_CSSProperty::PageBreakBefore:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_ePageBreakBefore =
              ToPageBreak(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::PageBreakAfter:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_ePageBreakAfter =
              ToPageBreak(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::PageBreakInside:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_ePageBreakInside =
              ToPageBreak(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::OverflowX:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_eOverflowX = ToOverflow(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::OverflowY:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_eOverflowY = ToOverflow(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::LineBreak:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_eLineBreak = ToLineBreak(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::ColumnCount:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_ColumnCount.Set(FDE_CSSLengthUnit::Auto);
        } else if (eType == FDE_CSSPrimitiveType::Number) {
          FDE_CSSNONINHERITS.m_ColumnCount.Set(FDE_CSSLengthUnit::Point,
                                               pPrimitive->GetFloat());
        }
        break;
      case FDE_CSSProperty::ColumnGap:
        SetLengthWithPercent(FDE_CSSNONINHERITS.m_ColumnGap, eType, pPrimitive,
                             FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::ColumnRuleColor:
        if (eType == FDE_CSSPrimitiveType::RGB) {
          FDE_CSSNONINHERITS.m_dwColumnRuleColor = pPrimitive->GetRGBColor();
          FDE_CSSNONINHERITS.m_bColumnRuleColorSame = false;
        }
        break;
      case FDE_CSSProperty::ColumnRuleStyle:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_eColumnRuleStyle =
              ToBorderStyle(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::ColumnRuleWidth:
        SetLengthWithPercent(FDE_CSSNONINHERITS.m_ColumnRuleWidth, eType,
                             pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::ColumnWidth:
        SetLengthWithPercent(FDE_CSSNONINHERITS.m_ColumnWidth, eType,
                             pPrimitive, FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::BackgroundImage:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_pszBKGImage = nullptr;
        } else if (eType == FDE_CSSPrimitiveType::URI) {
          int32_t iLength;
          FDE_CSSNONINHERITS.m_pszBKGImage = pPrimitive->GetString(iLength);
        }
        break;
      case FDE_CSSProperty::Position:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_ePosition = ToPosition(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::Top:
        SetLengthWithPercent(FDE_CSSNONINHERITS.m_Top, eType, pPrimitive,
                             FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::Bottom:
        SetLengthWithPercent(FDE_CSSNONINHERITS.m_Bottom, eType, pPrimitive,
                             FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::Left:
        SetLengthWithPercent(FDE_CSSNONINHERITS.m_Left, eType, pPrimitive,
                             FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::Right:
        SetLengthWithPercent(FDE_CSSNONINHERITS.m_Right, eType, pPrimitive,
                             FDE_CSSFONTSIZE);
        break;
      case FDE_CSSProperty::ListStyleImage:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_pszListStyleImage = nullptr;
        } else if (eType == FDE_CSSPrimitiveType::URI) {
          int32_t iLength;
          FDE_CSSINHERITS.m_pszListStyleImage = pPrimitive->GetString(iLength);
        }
        break;
      case FDE_CSSProperty::CaptionSide:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_eCaptionSide = ToCaptionSide(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::BackgroundRepeat:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_eBKGRepeat = ToBKGRepeat(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::BackgroundAttachment:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSNONINHERITS.m_eBKGAttachment =
              ToBKGAttachment(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::RubyAlign:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_eRubyAlign = ToRubyAlign(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::RubyOverhang:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_eRubyOverhang =
              ToRubyOverhang(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::RubyPosition:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          FDE_CSSINHERITS.m_eRubyPosition =
              ToRubyPosition(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::RubySpan:
        FDE_CSSNONINHERITS.m_pRubySpan = pPrimitive;
        break;
      default:
        break;
    }
  } else if (pValue->GetType() == FDE_CSSVALUETYPE_List) {
    IFDE_CSSValueList* pList = static_cast<IFDE_CSSValueList*>(pValue);
    int32_t iCount = pList->CountValues();
    if (iCount > 0) {
      switch (eProperty) {
        case FDE_CSSProperty::FontFamily:
          FDE_CSSINHERITS.m_pFontFamily = pList;
          break;
        case FDE_CSSProperty::TextDecoration:
          FDE_CSSNONINHERITS.m_dwTextDecoration = ToTextDecoration(pList);
          break;
        case FDE_CSSProperty::CounterIncrement: {
          if (!FDE_CSSNONINHERITS.m_pCounterStyle)
            FDE_CSSNONINHERITS.m_pCounterStyle = new CFDE_CSSCounterStyle;
          FDE_CSSNONINHERITS.m_pCounterStyle->SetCounterIncrementList(pList);
        } break;
        case FDE_CSSProperty::CounterReset: {
          if (!FDE_CSSNONINHERITS.m_pCounterStyle)
            FDE_CSSNONINHERITS.m_pCounterStyle = new CFDE_CSSCounterStyle;
          FDE_CSSNONINHERITS.m_pCounterStyle->SetCounterResetList(pList);
        } break;
        case FDE_CSSProperty::Content:
          FDE_CSSNONINHERITS.m_pContentList = pList;
          break;
        case FDE_CSSProperty::Quotes:
          FDE_CSSINHERITS.m_pQuotes = pList;
          break;
        case FDE_CSSProperty::TextCombine: {
          for (int32_t i = 0; i < pList->CountValues(); i++) {
            IFDE_CSSPrimitiveValue* pVal =
                static_cast<IFDE_CSSPrimitiveValue*>(pList->GetValue(i));
            switch (pVal->GetPrimitiveType()) {
              case FDE_CSSPrimitiveType::Enum: {
                switch (pVal->GetEnum()) {
                  case FDE_CSSPropertyValue::None: {
                    FDE_CSSNONINHERITS.m_eTextCombine =
                        FDE_CSSTextCombine::None;
                    FDE_CSSNONINHERITS.m_bHasTextCombineNumber = false;
                  } break;
                  case FDE_CSSPropertyValue::Horizontal: {
                    FDE_CSSNONINHERITS.m_eTextCombine =
                        FDE_CSSTextCombine::Horizontal;
                    FDE_CSSNONINHERITS.m_bHasTextCombineNumber = false;
                  } break;
                  default:
                    break;
                }
              } break;
              case FDE_CSSPrimitiveType::Number:
                FDE_CSSNONINHERITS.m_fTextCombineNumber = pVal->GetFloat();
                break;
              default:
                break;
            }
          }
        } break;
        case FDE_CSSProperty::TextEmphasisStyle: {
          FDE_CSSTextEmphasisFill eFill;
          FDE_CSSTextEmphasisMark eMark;
          for (int32_t i = 0; i < pList->CountValues(); i++) {
            IFDE_CSSPrimitiveValue* pVal =
                static_cast<IFDE_CSSPrimitiveValue*>(pList->GetValue(i));
            switch (pVal->GetPrimitiveType()) {
              case FDE_CSSPrimitiveType::Enum: {
                if (ToTextEmphasisFill(pVal->GetEnum(), eFill)) {
                  FDE_CSSINHERITS.m_eTextEmphasisFill = eFill;
                  continue;
                } else if (ToTextEmphasisMark(pVal->GetEnum(), eMark)) {
                  FDE_CSSINHERITS.m_eTextEmphasisMark = eMark;
                }
              } break;
              case FDE_CSSPrimitiveType::String: {
                FDE_CSSINHERITS.m_eTextEmphasisMark =
                    FDE_CSSTextEmphasisMark::Custom;
                int32_t iLen;
                FDE_CSSINHERITS.m_pszTextEmphasisCustomMark =
                    pVal->GetString(iLen);
              } break;
              default:
                break;
            }
          }
        } break;
        default:
          break;
      }
    }
  } else {
    ASSERT(false);
  }
}

FX_FLOAT CFDE_CSSStyleSelector::ApplyNumber(FDE_CSSPrimitiveType eUnit,
                                            FX_FLOAT fValue,
                                            FX_FLOAT fPercentBase) {
  switch (eUnit) {
    case FDE_CSSPrimitiveType::Pixels:
    case FDE_CSSPrimitiveType::Number:
      return fValue * 72 / 96;
    case FDE_CSSPrimitiveType::Points:
      return fValue;
    case FDE_CSSPrimitiveType::EMS:
    case FDE_CSSPrimitiveType::EXS:
      return fValue * fPercentBase;
    case FDE_CSSPrimitiveType::Percent:
      return fValue * fPercentBase / 100.0f;
    case FDE_CSSPrimitiveType::CentiMeters:
      return fValue * 28.3464f;
    case FDE_CSSPrimitiveType::MilliMeters:
      return fValue * 2.8346f;
    case FDE_CSSPrimitiveType::Inches:
      return fValue * 72.0f;
    case FDE_CSSPrimitiveType::Picas:
      return fValue / 12.0f;
    default:
      return fValue;
  }
}

FDE_CSSRubySpan CFDE_CSSStyleSelector::ToRubySpan(FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::None:
    default:
      return FDE_CSSRubySpan::None;
  }
}

FDE_CSSRubyPosition CFDE_CSSStyleSelector::ToRubyPosition(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Before:
      return FDE_CSSRubyPosition::Before;
    case FDE_CSSPropertyValue::After:
      return FDE_CSSRubyPosition::After;
    case FDE_CSSPropertyValue::Right:
      return FDE_CSSRubyPosition::Right;
    case FDE_CSSPropertyValue::Inline:
      return FDE_CSSRubyPosition::Inline;
    default:
      return FDE_CSSRubyPosition::Before;
  }
}

FDE_CSSRubyOverhang CFDE_CSSStyleSelector::ToRubyOverhang(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Auto:
      return FDE_CSSRubyOverhang::Auto;
    case FDE_CSSPropertyValue::Start:
      return FDE_CSSRubyOverhang::Start;
    case FDE_CSSPropertyValue::End:
      return FDE_CSSRubyOverhang::End;
    case FDE_CSSPropertyValue::None:
    default:
      return FDE_CSSRubyOverhang::None;
  }
}

FDE_CSSRubyAlign CFDE_CSSStyleSelector::ToRubyAlign(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Auto:
      return FDE_CSSRubyAlign::Auto;
    case FDE_CSSPropertyValue::Start:
      return FDE_CSSRubyAlign::Start;
    case FDE_CSSPropertyValue::Left:
      return FDE_CSSRubyAlign::End;
    case FDE_CSSPropertyValue::Center:
      return FDE_CSSRubyAlign::Center;
    case FDE_CSSPropertyValue::End:
      return FDE_CSSRubyAlign::End;
    case FDE_CSSPropertyValue::Right:
      return FDE_CSSRubyAlign::Right;
    case FDE_CSSPropertyValue::DistributeLetter:
      return FDE_CSSRubyAlign::DistributeLetter;
    case FDE_CSSPropertyValue::DistributeSpace:
      return FDE_CSSRubyAlign::DistributeSpace;
    case FDE_CSSPropertyValue::LineEdge:
      return FDE_CSSRubyAlign::LineEdge;
    default:
      return FDE_CSSRubyAlign::Auto;
  }
}

bool CFDE_CSSStyleSelector::ToTextEmphasisMark(FDE_CSSPropertyValue eValue,
                                               FDE_CSSTextEmphasisMark& eMark) {
  switch (eValue) {
    case FDE_CSSPropertyValue::None:
      eMark = FDE_CSSTextEmphasisMark::None;
      return true;
    case FDE_CSSPropertyValue::Dot:
      eMark = FDE_CSSTextEmphasisMark::Dot;
      return true;
    case FDE_CSSPropertyValue::Circle:
      eMark = FDE_CSSTextEmphasisMark::Circle;
      return true;
    case FDE_CSSPropertyValue::DoubleCircle:
      eMark = FDE_CSSTextEmphasisMark::DoubleCircle;
      return true;
    case FDE_CSSPropertyValue::Triangle:
      eMark = FDE_CSSTextEmphasisMark::Triangle;
      return true;
    case FDE_CSSPropertyValue::Sesame:
      eMark = FDE_CSSTextEmphasisMark::Sesame;
      return true;
    default:
      return false;
  }
}

bool CFDE_CSSStyleSelector::ToTextEmphasisFill(FDE_CSSPropertyValue eValue,
                                               FDE_CSSTextEmphasisFill& eFill) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Filled:
      eFill = FDE_CSSTextEmphasisFill::Filled;
      return true;
    case FDE_CSSPropertyValue::Open:
      eFill = FDE_CSSTextEmphasisFill::Open;
      return true;
    default:
      return false;
  }
}

FDE_CSSBackgroundAttachment CFDE_CSSStyleSelector::ToBKGAttachment(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Fixed:
      return FDE_CSSBackgroundAttachment::Fixed;
    case FDE_CSSPropertyValue::Scroll:
      return FDE_CSSBackgroundAttachment::Scroll;
    default:
      return FDE_CSSBackgroundAttachment::Fixed;
  }
}

FDE_CSSCaptionSide CFDE_CSSStyleSelector::ToCaptionSide(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Top:
      return FDE_CSSCaptionSide::Top;
    case FDE_CSSPropertyValue::Bottom:
      return FDE_CSSCaptionSide::Bottom;
    case FDE_CSSPropertyValue::Left:
      return FDE_CSSCaptionSide::Left;
    case FDE_CSSPropertyValue::Right:
      return FDE_CSSCaptionSide::Right;
    case FDE_CSSPropertyValue::Before:
      return FDE_CSSCaptionSide::Before;
    case FDE_CSSPropertyValue::After:
      return FDE_CSSCaptionSide::After;
    default:
      return FDE_CSSCaptionSide::Top;
  }
}

FDE_CSSPosition CFDE_CSSStyleSelector::ToPosition(FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Static:
      return FDE_CSSPosition::Static;
    case FDE_CSSPropertyValue::Relative:
      return FDE_CSSPosition::Relative;
    case FDE_CSSPropertyValue::Fixed:
      return FDE_CSSPosition::Fixed;
    case FDE_CSSPropertyValue::Absolute:
      return FDE_CSSPosition::Absolute;
    default:
      return FDE_CSSPosition::Static;
  }
}

FDE_CSSCursor CFDE_CSSStyleSelector::ToCursor(FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Auto:
      return FDE_CSSCursor::Auto;
    case FDE_CSSPropertyValue::Crosshair:
      return FDE_CSSCursor::Crosshair;
    case FDE_CSSPropertyValue::Default:
      return FDE_CSSCursor::Default;
    case FDE_CSSPropertyValue::Pointer:
      return FDE_CSSCursor::Pointer;
    case FDE_CSSPropertyValue::Move:
      return FDE_CSSCursor::Move;
    case FDE_CSSPropertyValue::EResize:
      return FDE_CSSCursor::EResize;
    case FDE_CSSPropertyValue::NeResize:
      return FDE_CSSCursor::NeResize;
    case FDE_CSSPropertyValue::NwResize:
      return FDE_CSSCursor::NwResize;
    case FDE_CSSPropertyValue::NResize:
      return FDE_CSSCursor::NResize;
    case FDE_CSSPropertyValue::SeResize:
      return FDE_CSSCursor::SeResize;
    case FDE_CSSPropertyValue::SwResize:
      return FDE_CSSCursor::SwResize;
    default:
      return FDE_CSSCursor::Auto;
  }
}

FDE_CSSBackgroundRepeat CFDE_CSSStyleSelector::ToBKGRepeat(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Repeat:
      return FDE_CSSBackgroundRepeat::Repeat;
    case FDE_CSSPropertyValue::RepeatX:
      return FDE_CSSBackgroundRepeat::RepeatX;
    case FDE_CSSPropertyValue::RepeatY:
      return FDE_CSSBackgroundRepeat::RepeatY;
    case FDE_CSSPropertyValue::NoRepeat:
      return FDE_CSSBackgroundRepeat::NoRepeat;
    default:
      return FDE_CSSBackgroundRepeat::Repeat;
  }
}

FDE_CSSTextCombine CFDE_CSSStyleSelector::ToTextCombine(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Horizontal:
      return FDE_CSSTextCombine::Horizontal;
    case FDE_CSSPropertyValue::None:
    default:
      return FDE_CSSTextCombine::None;
  }
}

FDE_CSSLineBreak CFDE_CSSStyleSelector::ToLineBreak(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Auto:
      return FDE_CSSLineBreak::Auto;
    case FDE_CSSPropertyValue::Loose:
      return FDE_CSSLineBreak::Loose;
    case FDE_CSSPropertyValue::Normal:
      return FDE_CSSLineBreak::Normal;
    case FDE_CSSPropertyValue::Strict:
      return FDE_CSSLineBreak::Strict;
    default:
      return FDE_CSSLineBreak::Auto;
  }
}

FDE_CSSOverFlow CFDE_CSSStyleSelector::ToOverflow(FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Visible:
      return FDE_CSSOverFlow::Visible;
    case FDE_CSSPropertyValue::Hidden:
      return FDE_CSSOverFlow::Hidden;
    case FDE_CSSPropertyValue::Scroll:
      return FDE_CSSOverFlow::Scroll;
    case FDE_CSSPropertyValue::Auto:
      return FDE_CSSOverFlow::Auto;
    case FDE_CSSPropertyValue::NoDisplay:
      return FDE_CSSOverFlow::NoDisplay;
    case FDE_CSSPropertyValue::NoContent:
      return FDE_CSSOverFlow::NoContent;
    default:
      return FDE_CSSOverFlow::Visible;
  }
}

FDE_CSSWritingMode CFDE_CSSStyleSelector::ToWritingMode(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::HorizontalTb:
      return FDE_CSSWritingMode::HorizontalTb;
    case FDE_CSSPropertyValue::VerticalRl:
      return FDE_CSSWritingMode::VerticalRl;
    case FDE_CSSPropertyValue::VerticalLr:
      return FDE_CSSWritingMode::VerticalLr;
    default:
      return FDE_CSSWritingMode::HorizontalTb;
  }
}

FDE_CSSWordBreak CFDE_CSSStyleSelector::ToWordBreak(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Normal:
      return FDE_CSSWordBreak::Normal;
    case FDE_CSSPropertyValue::KeepAll:
      return FDE_CSSWordBreak::KeepAll;
    case FDE_CSSPropertyValue::BreakAll:
      return FDE_CSSWordBreak::BreakAll;
    case FDE_CSSPropertyValue::KeepWords:
      return FDE_CSSWordBreak::KeepWords;
    default:
      return FDE_CSSWordBreak::Normal;
  }
}

FDE_CSSFloat CFDE_CSSStyleSelector::ToFloat(FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Left:
      return FDE_CSSFloat::Left;
    case FDE_CSSPropertyValue::Right:
      return FDE_CSSFloat::Right;
    case FDE_CSSPropertyValue::None:
      return FDE_CSSFloat::None;
    default:
      return FDE_CSSFloat::None;
  }
}

FDE_CSSClear CFDE_CSSStyleSelector::ToClear(FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::None:
      return FDE_CSSClear::None;
    case FDE_CSSPropertyValue::Left:
      return FDE_CSSClear::Left;
    case FDE_CSSPropertyValue::Right:
      return FDE_CSSClear::Right;
    case FDE_CSSPropertyValue::Both:
      return FDE_CSSClear::Both;
    default:
      return FDE_CSSClear::None;
  }
}

FDE_CSSPageBreak CFDE_CSSStyleSelector::ToPageBreak(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Avoid:
      return FDE_CSSPageBreak::Avoid;
    case FDE_CSSPropertyValue::Auto:
      return FDE_CSSPageBreak::Auto;
    case FDE_CSSPropertyValue::Always:
      return FDE_CSSPageBreak::Always;
    case FDE_CSSPropertyValue::Left:
      return FDE_CSSPageBreak::Left;
    case FDE_CSSPropertyValue::Right:
      return FDE_CSSPageBreak::Right;
    default:
      return FDE_CSSPageBreak::Auto;
  }
}

FDE_CSSDisplay CFDE_CSSStyleSelector::ToDisplay(FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Inline:
      return FDE_CSSDisplay::Inline;
    case FDE_CSSPropertyValue::Block:
      return FDE_CSSDisplay::Block;
    case FDE_CSSPropertyValue::None:
      return FDE_CSSDisplay::None;
    case FDE_CSSPropertyValue::ListItem:
      return FDE_CSSDisplay::ListItem;
    case FDE_CSSPropertyValue::TableCell:
      return FDE_CSSDisplay::TableCell;
    case FDE_CSSPropertyValue::TableRow:
      return FDE_CSSDisplay::TableRow;
    case FDE_CSSPropertyValue::Table:
      return FDE_CSSDisplay::Table;
    case FDE_CSSPropertyValue::TableCaption:
      return FDE_CSSDisplay::TableCaption;
    case FDE_CSSPropertyValue::TableRowGroup:
      return FDE_CSSDisplay::TableRowGroup;
    case FDE_CSSPropertyValue::TableHeaderGroup:
      return FDE_CSSDisplay::TableHeaderGroup;
    case FDE_CSSPropertyValue::TableFooterGroup:
      return FDE_CSSDisplay::TableFooterGroup;
    case FDE_CSSPropertyValue::TableColumnGroup:
      return FDE_CSSDisplay::TableColumnGroup;
    case FDE_CSSPropertyValue::TableColumn:
      return FDE_CSSDisplay::TableColumn;
    case FDE_CSSPropertyValue::InlineTable:
      return FDE_CSSDisplay::InlineTable;
    case FDE_CSSPropertyValue::InlineBlock:
      return FDE_CSSDisplay::InlineBlock;
    case FDE_CSSPropertyValue::RunIn:
      return FDE_CSSDisplay::RunIn;
    case FDE_CSSPropertyValue::Ruby:
      return FDE_CSSDisplay::Ruby;
    case FDE_CSSPropertyValue::RubyBase:
      return FDE_CSSDisplay::RubyBase;
    case FDE_CSSPropertyValue::RubyText:
      return FDE_CSSDisplay::RubyText;
    case FDE_CSSPropertyValue::RubyBaseGroup:
      return FDE_CSSDisplay::RubyBaseGroup;
    case FDE_CSSPropertyValue::RubyTextGroup:
      return FDE_CSSDisplay::RubyTextGroup;
    default:
      return FDE_CSSDisplay::Inline;
  }
}

FDE_CSSTextAlign CFDE_CSSStyleSelector::ToTextAlign(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Left:
      return FDE_CSSTextAlign::Left;
    case FDE_CSSPropertyValue::Center:
      return FDE_CSSTextAlign::Center;
    case FDE_CSSPropertyValue::Right:
      return FDE_CSSTextAlign::Right;
    case FDE_CSSPropertyValue::Justify:
      return FDE_CSSTextAlign::Justify;
    default:
      return FDE_CSSTextAlign::Left;
  }
}

uint16_t CFDE_CSSStyleSelector::ToFontWeight(FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Normal:
      return 400;
    case FDE_CSSPropertyValue::Bold:
      return 700;
    case FDE_CSSPropertyValue::Bolder:
      return 900;
    case FDE_CSSPropertyValue::Lighter:
      return 200;
    default:
      return 400;
  }
}

FDE_CSSFontStyle CFDE_CSSStyleSelector::ToFontStyle(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Italic:
    case FDE_CSSPropertyValue::Oblique:
      return FDE_CSSFontStyle::Italic;
    default:
      return FDE_CSSFontStyle::Normal;
  }
}

FDE_CSSBorderStyle CFDE_CSSStyleSelector::ToBorderStyle(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::None:
      return FDE_CSSBorderStyle::None;
    case FDE_CSSPropertyValue::Solid:
      return FDE_CSSBorderStyle::Solid;
    case FDE_CSSPropertyValue::Hidden:
      return FDE_CSSBorderStyle::Hidden;
    case FDE_CSSPropertyValue::Dotted:
      return FDE_CSSBorderStyle::Dotted;
    case FDE_CSSPropertyValue::Dashed:
      return FDE_CSSBorderStyle::Dashed;
    case FDE_CSSPropertyValue::Double:
      return FDE_CSSBorderStyle::Double;
    case FDE_CSSPropertyValue::Groove:
      return FDE_CSSBorderStyle::Groove;
    case FDE_CSSPropertyValue::Ridge:
      return FDE_CSSBorderStyle::Ridge;
    case FDE_CSSPropertyValue::Inset:
      return FDE_CSSBorderStyle::Inset;
    case FDE_CSSPropertyValue::Outset:
      return FDE_CSSBorderStyle::outset;
    default:
      return FDE_CSSBorderStyle::None;
  }
}

bool CFDE_CSSStyleSelector::SetLengthWithPercent(
    FDE_CSSLENGTH& width,
    FDE_CSSPrimitiveType eType,
    IFDE_CSSPrimitiveValue* pPrimitive,
    FX_FLOAT fFontSize) {
  if (eType == FDE_CSSPrimitiveType::Percent) {
    width.Set(FDE_CSSLengthUnit::Percent, pPrimitive->GetFloat() / 100.0f);
    return width.NonZero();
  } else if (eType >= FDE_CSSPrimitiveType::Number &&
             eType <= FDE_CSSPrimitiveType::Picas) {
    FX_FLOAT fValue = ApplyNumber(eType, pPrimitive->GetFloat(), fFontSize);
    width.Set(FDE_CSSLengthUnit::Point, fValue);
    return width.NonZero();
  } else if (eType == FDE_CSSPrimitiveType::Enum) {
    switch (pPrimitive->GetEnum()) {
      case FDE_CSSPropertyValue::Auto:
        width.Set(FDE_CSSLengthUnit::Auto);
        return true;
      case FDE_CSSPropertyValue::None:
        width.Set(FDE_CSSLengthUnit::None);
        return true;
      case FDE_CSSPropertyValue::Thin:
        width.Set(FDE_CSSLengthUnit::Point, 2);
        return true;
      case FDE_CSSPropertyValue::Medium:
        width.Set(FDE_CSSLengthUnit::Point, 3);
        return true;
      case FDE_CSSPropertyValue::Thick:
        width.Set(FDE_CSSLengthUnit::Point, 4);
        return true;
      default:
        return false;
    }
  }
  return false;
}

FX_FLOAT CFDE_CSSStyleSelector::ToFontSize(FDE_CSSPropertyValue eValue,
                                           FX_FLOAT fCurFontSize) {
  switch (eValue) {
    case FDE_CSSPropertyValue::XxSmall:
      return m_fDefFontSize / 1.2f / 1.2f / 1.2f;
    case FDE_CSSPropertyValue::XSmall:
      return m_fDefFontSize / 1.2f / 1.2f;
    case FDE_CSSPropertyValue::Small:
      return m_fDefFontSize / 1.2f;
    case FDE_CSSPropertyValue::Medium:
      return m_fDefFontSize;
    case FDE_CSSPropertyValue::Large:
      return m_fDefFontSize * 1.2f;
    case FDE_CSSPropertyValue::XLarge:
      return m_fDefFontSize * 1.2f * 1.2f;
    case FDE_CSSPropertyValue::XxLarge:
      return m_fDefFontSize * 1.2f * 1.2f * 1.2f;
    case FDE_CSSPropertyValue::Larger:
      return fCurFontSize * 1.2f;
    case FDE_CSSPropertyValue::Smaller:
      return fCurFontSize / 1.2f;
    default:
      return fCurFontSize;
  }
}

FDE_CSSVerticalAlign CFDE_CSSStyleSelector::ToVerticalAlign(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Baseline:
      return FDE_CSSVerticalAlign::Baseline;
    case FDE_CSSPropertyValue::Middle:
      return FDE_CSSVerticalAlign::Middle;
    case FDE_CSSPropertyValue::Bottom:
      return FDE_CSSVerticalAlign::Bottom;
    case FDE_CSSPropertyValue::Super:
      return FDE_CSSVerticalAlign::Super;
    case FDE_CSSPropertyValue::Sub:
      return FDE_CSSVerticalAlign::Sub;
    case FDE_CSSPropertyValue::Top:
      return FDE_CSSVerticalAlign::Top;
    case FDE_CSSPropertyValue::TextTop:
      return FDE_CSSVerticalAlign::TextTop;
    case FDE_CSSPropertyValue::TextBottom:
      return FDE_CSSVerticalAlign::TextBottom;
    default:
      return FDE_CSSVerticalAlign::Baseline;
  }
}

FDE_CSSListStyleType CFDE_CSSStyleSelector::ToListStyleType(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::None:
      return FDE_CSSListStyleType::None;
    case FDE_CSSPropertyValue::Disc:
      return FDE_CSSListStyleType::Disc;
    case FDE_CSSPropertyValue::Circle:
      return FDE_CSSListStyleType::Circle;
    case FDE_CSSPropertyValue::Square:
      return FDE_CSSListStyleType::Square;
    case FDE_CSSPropertyValue::Decimal:
      return FDE_CSSListStyleType::Decimal;
    case FDE_CSSPropertyValue::DecimalLeadingZero:
      return FDE_CSSListStyleType::DecimalLeadingZero;
    case FDE_CSSPropertyValue::LowerRoman:
      return FDE_CSSListStyleType::LowerRoman;
    case FDE_CSSPropertyValue::UpperRoman:
      return FDE_CSSListStyleType::UpperRoman;
    case FDE_CSSPropertyValue::LowerGreek:
      return FDE_CSSListStyleType::LowerGreek;
    case FDE_CSSPropertyValue::LowerLatin:
      return FDE_CSSListStyleType::LowerLatin;
    case FDE_CSSPropertyValue::UpperLatin:
      return FDE_CSSListStyleType::UpperLatin;
    case FDE_CSSPropertyValue::Armenian:
      return FDE_CSSListStyleType::Armenian;
    case FDE_CSSPropertyValue::Georgian:
      return FDE_CSSListStyleType::Georgian;
    case FDE_CSSPropertyValue::LowerAlpha:
      return FDE_CSSListStyleType::LowerAlpha;
    case FDE_CSSPropertyValue::UpperAlpha:
      return FDE_CSSListStyleType::UpperAlpha;
    case FDE_CSSPropertyValue::CjkIdeographic:
      return FDE_CSSListStyleType::CjkIdeographic;
    case FDE_CSSPropertyValue::Hebrew:
      return FDE_CSSListStyleType::Hebrew;
    case FDE_CSSPropertyValue::Hiragana:
      return FDE_CSSListStyleType::Hiragana;
    case FDE_CSSPropertyValue::HiraganaIroha:
      return FDE_CSSListStyleType::HiraganaIroha;
    case FDE_CSSPropertyValue::Katakana:
      return FDE_CSSListStyleType::Katakana;
    case FDE_CSSPropertyValue::KatakanaIroha:
      return FDE_CSSListStyleType::KatakanaIroha;
    default:
      return FDE_CSSListStyleType::Disc;
  }
}

FDE_CSSListStylePosition CFDE_CSSStyleSelector::ToListStylePosition(
    FDE_CSSPropertyValue eValue) {
  return eValue == FDE_CSSPropertyValue::Inside
             ? FDE_CSSListStylePosition::Inside
             : FDE_CSSListStylePosition::Outside;
}

FDE_CSSVisibility CFDE_CSSStyleSelector::ToVisibility(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Visible:
      return FDE_CSSVisibility::Visible;
    case FDE_CSSPropertyValue::Hidden:
      return FDE_CSSVisibility::Hidden;
    case FDE_CSSPropertyValue::Collapse:
      return FDE_CSSVisibility::Collapse;
    default:
      return FDE_CSSVisibility::Visible;
  }
}

FDE_CSSWhitespace CFDE_CSSStyleSelector::ToWhiteSpace(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Normal:
      return FDE_CSSWhitespace::Normal;
    case FDE_CSSPropertyValue::Pre:
      return FDE_CSSWhitespace::Pre;
    case FDE_CSSPropertyValue::Nowrap:
      return FDE_CSSWhitespace::Nowrap;
    case FDE_CSSPropertyValue::PreWrap:
      return FDE_CSSWhitespace::PreWrap;
    case FDE_CSSPropertyValue::PreLine:
      return FDE_CSSWhitespace::PreLine;
    default:
      return FDE_CSSWhitespace::Normal;
  }
}

uint32_t CFDE_CSSStyleSelector::ToTextDecoration(IFDE_CSSValueList* pValue) {
  uint32_t dwDecoration = 0;
  for (int32_t i = pValue->CountValues() - 1; i >= 0; --i) {
    IFDE_CSSPrimitiveValue* pPrimitive =
        static_cast<IFDE_CSSPrimitiveValue*>(pValue->GetValue(i));
    if (pPrimitive->GetPrimitiveType() == FDE_CSSPrimitiveType::Enum) {
      switch (pPrimitive->GetEnum()) {
        case FDE_CSSPropertyValue::Underline:
          dwDecoration |= FDE_CSSTEXTDECORATION_Underline;
          break;
        case FDE_CSSPropertyValue::LineThrough:
          dwDecoration |= FDE_CSSTEXTDECORATION_LineThrough;
          break;
        case FDE_CSSPropertyValue::Overline:
          dwDecoration |= FDE_CSSTEXTDECORATION_Overline;
          break;
        case FDE_CSSPropertyValue::Blink:
          dwDecoration |= FDE_CSSTEXTDECORATION_Blink;
          break;
        case FDE_CSSPropertyValue::Double:
          dwDecoration |= FDE_CSSTEXTDECORATION_Double;
          break;
        default:
          break;
      }
    }
  }
  return dwDecoration;
}

FDE_CSSTextTransform CFDE_CSSStyleSelector::ToTextTransform(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::None:
      return FDE_CSSTextTransform::None;
    case FDE_CSSPropertyValue::Capitalize:
      return FDE_CSSTextTransform::Capitalize;
    case FDE_CSSPropertyValue::Uppercase:
      return FDE_CSSTextTransform::UpperCase;
    case FDE_CSSPropertyValue::Lowercase:
      return FDE_CSSTextTransform::LowerCase;
    default:
      return FDE_CSSTextTransform::None;
  }
}

FDE_CSSFontVariant CFDE_CSSStyleSelector::ToFontVariant(
    FDE_CSSPropertyValue eValue) {
  return eValue == FDE_CSSPropertyValue::SmallCaps
             ? FDE_CSSFontVariant::SmallCaps
             : FDE_CSSFontVariant::Normal;
}

CFDE_CSSComputedStyle::CFDE_CSSComputedStyle() : m_dwRefCount(1) {}

CFDE_CSSComputedStyle::~CFDE_CSSComputedStyle() {}

uint32_t CFDE_CSSComputedStyle::Retain() {
  return ++m_dwRefCount;
}

uint32_t CFDE_CSSComputedStyle::Release() {
  uint32_t dwRefCount = --m_dwRefCount;
  if (dwRefCount == 0) {
    delete m_NonInheritedData.m_pCounterStyle;
    delete this;
  }
  return dwRefCount;
}

void CFDE_CSSComputedStyle::Reset() {
  m_InheritedData.Reset();
  m_NonInheritedData.Reset();
}

IFDE_CSSFontStyle* CFDE_CSSComputedStyle::GetFontStyles() {
  return static_cast<IFDE_CSSFontStyle*>(this);
}

IFDE_CSSBoundaryStyle* CFDE_CSSComputedStyle::GetBoundaryStyles() {
  return static_cast<IFDE_CSSBoundaryStyle*>(this);
}

IFDE_CSSPositionStyle* CFDE_CSSComputedStyle::GetPositionStyles() {
  return static_cast<IFDE_CSSPositionStyle*>(this);
}

IFDE_CSSParagraphStyle* CFDE_CSSComputedStyle::GetParagraphStyles() {
  return static_cast<IFDE_CSSParagraphStyle*>(this);
}

bool CFDE_CSSComputedStyle::GetCustomStyle(const CFX_WideStringC& wsName,
                                           CFX_WideString& wsValue) const {
  for (int32_t i = pdfium::CollectionSize<int32_t>(m_CustomProperties) - 2;
       i > -1; i -= 2) {
    if (wsName == m_CustomProperties[i]) {
      wsValue = m_CustomProperties[i + 1];
      return true;
    }
  }
  return false;
}

int32_t CFDE_CSSComputedStyle::CountFontFamilies() const {
  return m_InheritedData.m_pFontFamily
             ? m_InheritedData.m_pFontFamily->CountValues()
             : 0;
}

const FX_WCHAR* CFDE_CSSComputedStyle::GetFontFamily(int32_t index) const {
  return (static_cast<IFDE_CSSPrimitiveValue*>(
              m_InheritedData.m_pFontFamily->GetValue(index)))
      ->GetString(index);
}

uint16_t CFDE_CSSComputedStyle::GetFontWeight() const {
  return m_InheritedData.m_wFontWeight;
}

FDE_CSSFontVariant CFDE_CSSComputedStyle::GetFontVariant() const {
  return static_cast<FDE_CSSFontVariant>(m_InheritedData.m_eFontVariant);
}

FDE_CSSFontStyle CFDE_CSSComputedStyle::GetFontStyle() const {
  return static_cast<FDE_CSSFontStyle>(m_InheritedData.m_eFontStyle);
}

FX_FLOAT CFDE_CSSComputedStyle::GetFontSize() const {
  return m_InheritedData.m_fFontSize;
}

FX_ARGB CFDE_CSSComputedStyle::GetColor() const {
  return m_InheritedData.m_dwFontColor;
}

void CFDE_CSSComputedStyle::SetFontWeight(uint16_t wFontWeight) {
  m_InheritedData.m_wFontWeight = wFontWeight;
}

void CFDE_CSSComputedStyle::SetFontVariant(FDE_CSSFontVariant eFontVariant) {
  m_InheritedData.m_eFontVariant = eFontVariant;
}

void CFDE_CSSComputedStyle::SetFontStyle(FDE_CSSFontStyle eFontStyle) {
  m_InheritedData.m_eFontStyle = eFontStyle;
}

void CFDE_CSSComputedStyle::SetFontSize(FX_FLOAT fFontSize) {
  m_InheritedData.m_fFontSize = fFontSize;
}

void CFDE_CSSComputedStyle::SetColor(FX_ARGB dwFontColor) {
  m_InheritedData.m_dwFontColor = dwFontColor;
}

const FDE_CSSRECT* CFDE_CSSComputedStyle::GetBorderWidth() const {
  return m_NonInheritedData.m_bHasBorder ? &(m_NonInheritedData.m_BorderWidth)
                                         : nullptr;
}

const FDE_CSSRECT* CFDE_CSSComputedStyle::GetMarginWidth() const {
  return m_NonInheritedData.m_bHasMargin ? &(m_NonInheritedData.m_MarginWidth)
                                         : nullptr;
}

const FDE_CSSRECT* CFDE_CSSComputedStyle::GetPaddingWidth() const {
  return m_NonInheritedData.m_bHasPadding ? &(m_NonInheritedData.m_PaddingWidth)
                                          : nullptr;
}

void CFDE_CSSComputedStyle::SetMarginWidth(const FDE_CSSRECT& rect) {
  m_NonInheritedData.m_MarginWidth = rect;
  m_NonInheritedData.m_bHasMargin = true;
}

void CFDE_CSSComputedStyle::SetPaddingWidth(const FDE_CSSRECT& rect) {
  m_NonInheritedData.m_PaddingWidth = rect;
  m_NonInheritedData.m_bHasPadding = true;
}

FDE_CSSDisplay CFDE_CSSComputedStyle::GetDisplay() const {
  return static_cast<FDE_CSSDisplay>(m_NonInheritedData.m_eDisplay);
}

FX_FLOAT CFDE_CSSComputedStyle::GetLineHeight() const {
  return m_InheritedData.m_fLineHeight;
}

const FDE_CSSLENGTH& CFDE_CSSComputedStyle::GetTextIndent() const {
  return m_InheritedData.m_TextIndent;
}

FDE_CSSTextAlign CFDE_CSSComputedStyle::GetTextAlign() const {
  return static_cast<FDE_CSSTextAlign>(m_InheritedData.m_eTextAlign);
}

FDE_CSSVerticalAlign CFDE_CSSComputedStyle::GetVerticalAlign() const {
  return static_cast<FDE_CSSVerticalAlign>(m_NonInheritedData.m_eVerticalAlign);
}

FX_FLOAT CFDE_CSSComputedStyle::GetNumberVerticalAlign() const {
  return m_NonInheritedData.m_fVerticalAlign;
}

uint32_t CFDE_CSSComputedStyle::GetTextDecoration() const {
  return m_NonInheritedData.m_dwTextDecoration;
}

const FDE_CSSLENGTH& CFDE_CSSComputedStyle::GetLetterSpacing() const {
  return m_InheritedData.m_LetterSpacing;
}

void CFDE_CSSComputedStyle::SetLineHeight(FX_FLOAT fLineHeight) {
  m_InheritedData.m_fLineHeight = fLineHeight;
}

void CFDE_CSSComputedStyle::SetTextIndent(const FDE_CSSLENGTH& textIndent) {
  m_InheritedData.m_TextIndent = textIndent;
}

void CFDE_CSSComputedStyle::SetTextAlign(FDE_CSSTextAlign eTextAlign) {
  m_InheritedData.m_eTextAlign = eTextAlign;
}

void CFDE_CSSComputedStyle::SetNumberVerticalAlign(FX_FLOAT fAlign) {
  m_NonInheritedData.m_eVerticalAlign = FDE_CSSVerticalAlign::Number,
  m_NonInheritedData.m_fVerticalAlign = fAlign;
}

void CFDE_CSSComputedStyle::SetTextDecoration(uint32_t dwTextDecoration) {
  m_NonInheritedData.m_dwTextDecoration = dwTextDecoration;
}

void CFDE_CSSComputedStyle::SetLetterSpacing(
    const FDE_CSSLENGTH& letterSpacing) {
  m_InheritedData.m_LetterSpacing = letterSpacing;
}

void CFDE_CSSComputedStyle::AddCustomStyle(const CFX_WideString& wsName,
                                           const CFX_WideString& wsValue) {
  m_CustomProperties.push_back(wsName);
  m_CustomProperties.push_back(wsValue);
}

CFDE_CSSInheritedData::CFDE_CSSInheritedData() {
  Reset();
}

void CFDE_CSSInheritedData::Reset() {
  FXSYS_memset(this, 0, sizeof(CFDE_CSSInheritedData));
  m_LetterSpacing.Set(FDE_CSSLengthUnit::Normal);
  m_WordSpacing.Set(FDE_CSSLengthUnit::Normal);
  m_TextIndent.Set(FDE_CSSLengthUnit::Point, 0);
  m_fFontSize = 12.0f;
  m_fLineHeight = 14.0f;
  m_wFontWeight = 400;
  m_dwFontColor = 0xFF000000;
  m_iWidows = 2;
  m_bTextEmphasisColorCurrent = true;
  m_iOrphans = 2;
}

CFDE_CSSNonInheritedData::CFDE_CSSNonInheritedData() {
  Reset();
}

void CFDE_CSSNonInheritedData::Reset() {
  FXSYS_memset(this, 0, sizeof(CFDE_CSSNonInheritedData));
  m_PaddingWidth.Set(FDE_CSSLengthUnit::Point, 0);
  m_MarginWidth = m_PaddingWidth;
  m_BorderWidth = m_PaddingWidth;
  m_MinBoxSize.Set(FDE_CSSLengthUnit::Point, 0);
  m_MaxBoxSize.Set(FDE_CSSLengthUnit::None);
  m_eDisplay = FDE_CSSDisplay::Inline;
  m_fVerticalAlign = 0.0f;
  m_ColumnCount.Set(FDE_CSSLengthUnit::Auto);
  m_ColumnGap.Set(FDE_CSSLengthUnit::Normal);
  m_bColumnRuleColorSame = true;
  m_ColumnWidth.Set(FDE_CSSLengthUnit::Auto);
  m_ColumnRuleWidth.Set(FDE_CSSLengthUnit::Auto);
  m_eTextCombine = FDE_CSSTextCombine::None;
}
