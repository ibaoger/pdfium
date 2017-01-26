// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssstyleselector.h"

#include <algorithm>
#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fde/css/cfde_cssaccelerator.h"
#include "xfa/fde/css/cfde_csscolorvalue.h"
#include "xfa/fde/css/cfde_csscomputedstyle.h"
#include "xfa/fde/css/cfde_csscustomproperty.h"
#include "xfa/fde/css/cfde_cssdeclaration.h"
#include "xfa/fde/css/cfde_cssenumvalue.h"
#include "xfa/fde/css/cfde_csspropertyholder.h"
#include "xfa/fde/css/cfde_cssselector.h"
#include "xfa/fde/css/cfde_cssstylesheet.h"
#include "xfa/fde/css/cfde_csssyntaxparser.h"
#include "xfa/fde/css/cfde_csstagcache.h"
#include "xfa/fde/css/cfde_cssvaluelist.h"
#include "xfa/fxfa/app/cxfa_csstagprovider.h"

namespace {

template <class T>
T* ToValue(CFDE_CSSValue* val) {
  return static_cast<T*>(val);
}

template <class T>
const T* ToValue(const CFDE_CSSValue* val) {
  return static_cast<T*>(val);
}

}  // namespace

CFDE_CSSStyleSelector::CFDE_CSSStyleSelector(CFGAS_FontMgr* pFontMgr)
    : m_pFontMgr(pFontMgr), m_fDefFontSize(12.0f) {}

CFDE_CSSStyleSelector::~CFDE_CSSStyleSelector() {}

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

CFX_RetainPtr<CFDE_CSSComputedStyle> CFDE_CSSStyleSelector::CreateComputedStyle(
    CFDE_CSSComputedStyle* pParentStyle) {
  auto pStyle = pdfium::MakeRetain<CFDE_CSSComputedStyle>();
  if (pParentStyle)
    pStyle->m_InheritedData = pParentStyle->m_InheritedData;
  return pStyle;
}

void CFDE_CSSStyleSelector::SetUAStyleSheet(
    std::unique_ptr<CFDE_CSSStyleSheet> pSheet) {
  m_UAStyles = std::move(pSheet);
}

void CFDE_CSSStyleSelector::UpdateStyleIndex() {
  m_UARules.Clear();
  m_UARules.AddRulesFrom(m_UAStyles.get(), m_pFontMgr);
}

std::vector<const CFDE_CSSDeclaration*>
CFDE_CSSStyleSelector::MatchDeclarations(CXFA_CSSTagProvider* pTag) {
  ASSERT(pTag);

  CFDE_CSSTagCache* pCache = m_pAccelerator->top();
  ASSERT(pCache && pCache->GetTag() == pTag);

  std::vector<const CFDE_CSSDeclaration*> matchedDecls;
  if (m_UARules.CountSelectors() == 0 || !pCache->HashTag())
    return matchedDecls;

  auto rules = m_UARules.GetTagRuleData(pCache->HashTag());
  if (!rules)
    return matchedDecls;

  for (const auto& d : *rules) {
    if (MatchSelector(pCache, d->pSelector))
      matchedDecls.push_back(d->pDeclaration);
  }
  return matchedDecls;
}

bool CFDE_CSSStyleSelector::MatchSelector(CFDE_CSSTagCache* pCache,
                                          CFDE_CSSSelector* pSel) {
  uint32_t dwHash;
  while (pSel && pCache) {
    switch (pSel->GetType()) {
      case FDE_CSSSelectorType::Descendant:
        dwHash = pSel->GetNameHash();
        while ((pCache = pCache->GetParent()) != nullptr) {
          if (dwHash != pCache->HashTag())
            continue;
          if (MatchSelector(pCache, pSel->GetNextSelector()))
            return true;
        }
        return false;
      case FDE_CSSSelectorType::Element:
        dwHash = pSel->GetNameHash();
        if (dwHash != pCache->HashTag())
          return false;
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
    const std::vector<const CFDE_CSSDeclaration*>& declArray,
    CFDE_CSSComputedStyle* pDestStyle) {
  ASSERT(pDestStyle);

  static const uint32_t s_dwStyleHash = FX_HashCode_GetW(L"style", true);
  static const uint32_t s_dwAlignHash = FX_HashCode_GetW(L"align", true);

  if (!pTag->empty()) {
    std::unique_ptr<CFDE_CSSDeclaration> pDecl;
    for (auto it : *pTag) {
      CFX_WideString wsAttri = it.first;
      CFX_WideString wsValue = it.second;
      uint32_t dwAttriHash = FX_HashCode_GetW(wsAttri.AsStringC(), true);
      if (dwAttriHash == s_dwStyleHash) {
        if (!pDecl)
          pDecl = pdfium::MakeUnique<CFDE_CSSDeclaration>();

        AppendInlineStyle(pDecl.get(), wsValue.c_str(), wsValue.GetLength());
      } else if (dwAttriHash == s_dwAlignHash) {
        if (!pDecl)
          pDecl = pdfium::MakeUnique<CFDE_CSSDeclaration>();

        FDE_CSSPropertyArgs args;
        args.pStringCache = nullptr;
        args.pProperty = FDE_GetCSSPropertyByEnum(FDE_CSSProperty::TextAlign);
        pDecl->AddProperty(&args, wsValue.c_str(), wsValue.GetLength());
      }
    }

    if (pDecl) {
      ApplyDeclarations(declArray, pDecl.get(), pDestStyle);
      return;
    }
  }

  if (declArray.empty())
    return;

  ApplyDeclarations(declArray, nullptr, pDestStyle);
}

void CFDE_CSSStyleSelector::ApplyDeclarations(
    const std::vector<const CFDE_CSSDeclaration*>& declArray,
    const CFDE_CSSDeclaration* extraDecl,
    CFDE_CSSComputedStyle* pDestStyle) {
  CFDE_CSSComputedStyle* pComputedStyle = pDestStyle;

  std::vector<const CFDE_CSSPropertyHolder*> importants;
  std::vector<const CFDE_CSSPropertyHolder*> normals;
  std::vector<const CFDE_CSSCustomProperty*> customs;

  for (auto& decl : declArray)
    ExtractValues(decl, &importants, &normals, &customs);
  if (extraDecl)
    ExtractValues(extraDecl, &importants, &normals, &customs);

  for (auto& prop : normals)
    ApplyProperty(prop->eProperty, prop->pValue.Get(), pComputedStyle);
  for (auto& prop : customs)
    pComputedStyle->AddCustomStyle(prop->pwsName, prop->pwsValue);
  for (auto& prop : importants)
    ApplyProperty(prop->eProperty, prop->pValue.Get(), pComputedStyle);
}

void CFDE_CSSStyleSelector::ExtractValues(
    const CFDE_CSSDeclaration* decl,
    std::vector<const CFDE_CSSPropertyHolder*>* importants,
    std::vector<const CFDE_CSSPropertyHolder*>* normals,
    std::vector<const CFDE_CSSCustomProperty*>* custom) {
  for (const auto& holder : *decl) {
    if (holder->bImportant)
      importants->push_back(holder.get());
    else
      normals->push_back(holder.get());
  }
  for (auto it = decl->custom_begin(); it != decl->custom_end(); it++)
    custom->push_back(it->get());
}

void CFDE_CSSStyleSelector::AppendInlineStyle(CFDE_CSSDeclaration* pDecl,
                                              const FX_WCHAR* psz,
                                              int32_t iLen) {
  ASSERT(pDecl && psz && iLen > 0);
  auto pSyntax = pdfium::MakeUnique<CFDE_CSSSyntaxParser>();
  if (!pSyntax->Init(psz, iLen, 32, true))
    return;

  int32_t iLen2 = 0;
  const FX_WCHAR* psz2;
  FDE_CSSPropertyArgs args;
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

void CFDE_CSSStyleSelector::ApplyProperty(
    FDE_CSSProperty eProperty,
    CFDE_CSSValue* pValue,
    CFDE_CSSComputedStyle* pComputedStyle) {
  if (pValue->GetType() != FDE_CSSPrimitiveType::List) {
    FDE_CSSPrimitiveType eType = pValue->GetType();
    switch (eProperty) {
      case FDE_CSSProperty::Display:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_NonInheritedData.m_eDisplay =
              ToDisplay(ToValue<CFDE_CSSEnumValue>(pValue)->Value());
        }
        break;
      case FDE_CSSProperty::FontSize: {
        FX_FLOAT& fFontSize = pComputedStyle->m_InheritedData.m_fFontSize;
        if (eType == FDE_CSSPrimitiveType::Number) {
          fFontSize = ToValue<CFDE_CSSNumberValue>(pValue)->Apply(fFontSize);
        } else if (eType == FDE_CSSPrimitiveType::Enum) {
          fFontSize = ToFontSize(ToValue<CFDE_CSSEnumValue>(pValue)->Value(),
                                 fFontSize);
        }
      } break;
      case FDE_CSSProperty::LineHeight:
        if (eType == FDE_CSSPrimitiveType::Number) {
          const CFDE_CSSNumberValue* v = ToValue<CFDE_CSSNumberValue>(pValue);
          if (v->Kind() == FDE_CSSNumberType::Number) {
            pComputedStyle->m_InheritedData.m_fLineHeight =
                v->Value() * pComputedStyle->m_InheritedData.m_fFontSize;
          } else {
            pComputedStyle->m_InheritedData.m_fLineHeight =
                v->Apply(pComputedStyle->m_InheritedData.m_fFontSize);
          }
        }
        break;
      case FDE_CSSProperty::TextAlign:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_InheritedData.m_eTextAlign =
              ToTextAlign(ToValue<CFDE_CSSEnumValue>(pValue)->Value());
        }
        break;
      case FDE_CSSProperty::TextIndent:
        SetLengthWithPercent(pComputedStyle->m_InheritedData.m_TextIndent,
                             eType, pValue,
                             pComputedStyle->m_InheritedData.m_fFontSize);
        break;
      case FDE_CSSProperty::FontWeight:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_InheritedData.m_wFontWeight =
              ToFontWeight(ToValue<CFDE_CSSEnumValue>(pValue)->Value());
        } else if (eType == FDE_CSSPrimitiveType::Number) {
          int32_t iValue =
              (int32_t)ToValue<CFDE_CSSNumberValue>(pValue)->Value() / 100;
          if (iValue >= 1 && iValue <= 9) {
            pComputedStyle->m_InheritedData.m_wFontWeight = iValue * 100;
          }
        }
        break;
      case FDE_CSSProperty::FontStyle:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_InheritedData.m_eFontStyle =
              ToFontStyle(ToValue<CFDE_CSSEnumValue>(pValue)->Value());
        }
        break;
      case FDE_CSSProperty::Color:
        if (eType == FDE_CSSPrimitiveType::RGB) {
          pComputedStyle->m_InheritedData.m_dwFontColor =
              ToValue<CFDE_CSSColorValue>(pValue)->Value();
        }
        break;
      case FDE_CSSProperty::MarginLeft:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_MarginWidth.left, eType,
                pValue, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasMargin = true;
        }
        break;
      case FDE_CSSProperty::MarginTop:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_MarginWidth.top, eType,
                pValue, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasMargin = true;
        }
        break;
      case FDE_CSSProperty::MarginRight:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_MarginWidth.right, eType,
                pValue, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasMargin = true;
        }
        break;
      case FDE_CSSProperty::MarginBottom:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_MarginWidth.bottom, eType,
                pValue, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasMargin = true;
        }
        break;
      case FDE_CSSProperty::PaddingLeft:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_PaddingWidth.left, eType,
                pValue, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasPadding = true;
        }
        break;
      case FDE_CSSProperty::PaddingTop:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_PaddingWidth.top, eType,
                pValue, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasPadding = true;
        }
        break;
      case FDE_CSSProperty::PaddingRight:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_PaddingWidth.right, eType,
                pValue, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasPadding = true;
        }
        break;
      case FDE_CSSProperty::PaddingBottom:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_PaddingWidth.bottom, eType,
                pValue, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasPadding = true;
        }
        break;
      case FDE_CSSProperty::BorderLeftWidth:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_BorderWidth.left, eType,
                pValue, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasBorder = true;
        }
        break;
      case FDE_CSSProperty::BorderTopWidth:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_BorderWidth.top, eType,
                pValue, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasBorder = true;
        }
        break;
      case FDE_CSSProperty::BorderRightWidth:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_BorderWidth.right, eType,
                pValue, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasBorder = true;
        }
        break;
      case FDE_CSSProperty::BorderBottomWidth:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_BorderWidth.bottom, eType,
                pValue, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasBorder = true;
        }
        break;
      case FDE_CSSProperty::VerticalAlign:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_NonInheritedData.m_eVerticalAlign =
              ToVerticalAlign(ToValue<CFDE_CSSEnumValue>(pValue)->Value());
        } else if (eType == FDE_CSSPrimitiveType::Number) {
          pComputedStyle->m_NonInheritedData.m_eVerticalAlign =
              FDE_CSSVerticalAlign::Number;
          pComputedStyle->m_NonInheritedData.m_fVerticalAlign =
              ToValue<CFDE_CSSNumberValue>(pValue)->Apply(
                  pComputedStyle->m_InheritedData.m_fFontSize);
        }
        break;
      case FDE_CSSProperty::FontVariant:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_InheritedData.m_eFontVariant =
              ToFontVariant(ToValue<CFDE_CSSEnumValue>(pValue)->Value());
        }
        break;
      case FDE_CSSProperty::LetterSpacing:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_InheritedData.m_LetterSpacing.Set(
              FDE_CSSLengthUnit::Normal);
        } else if (eType == FDE_CSSPrimitiveType::Number) {
          if (ToValue<CFDE_CSSNumberValue>(pValue)->Kind() ==
              FDE_CSSNumberType::Percent) {
            break;
          }

          SetLengthWithPercent(pComputedStyle->m_InheritedData.m_LetterSpacing,
                               eType, pValue,
                               pComputedStyle->m_InheritedData.m_fFontSize);
        }
        break;
      case FDE_CSSProperty::WordSpacing:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_InheritedData.m_WordSpacing.Set(
              FDE_CSSLengthUnit::Normal);
        } else if (eType == FDE_CSSPrimitiveType::Number) {
          if (ToValue<CFDE_CSSNumberValue>(pValue)->Kind() ==
              FDE_CSSNumberType::Percent) {
            break;
          }
          SetLengthWithPercent(pComputedStyle->m_InheritedData.m_WordSpacing,
                               eType, pValue,
                               pComputedStyle->m_InheritedData.m_fFontSize);
        }
        break;
      case FDE_CSSProperty::Top:
        SetLengthWithPercent(pComputedStyle->m_NonInheritedData.m_Top, eType,
                             pValue,
                             pComputedStyle->m_InheritedData.m_fFontSize);
        break;
      case FDE_CSSProperty::Bottom:
        SetLengthWithPercent(pComputedStyle->m_NonInheritedData.m_Bottom, eType,
                             pValue,
                             pComputedStyle->m_InheritedData.m_fFontSize);
        break;
      case FDE_CSSProperty::Left:
        SetLengthWithPercent(pComputedStyle->m_NonInheritedData.m_Left, eType,
                             pValue,
                             pComputedStyle->m_InheritedData.m_fFontSize);
        break;
      case FDE_CSSProperty::Right:
        SetLengthWithPercent(pComputedStyle->m_NonInheritedData.m_Right, eType,
                             pValue,
                             pComputedStyle->m_InheritedData.m_fFontSize);
        break;
      default:
        break;
    }
  } else if (pValue->GetType() == FDE_CSSPrimitiveType::List) {
    CFDE_CSSValueList* pList = ToValue<CFDE_CSSValueList>(pValue);
    int32_t iCount = pList->CountValues();
    if (iCount > 0) {
      switch (eProperty) {
        case FDE_CSSProperty::FontFamily:
          pComputedStyle->m_InheritedData.m_pFontFamily = pList;
          break;
        case FDE_CSSProperty::TextDecoration:
          pComputedStyle->m_NonInheritedData.m_dwTextDecoration =
              ToTextDecoration(pList);
          break;
        default:
          break;
      }
    }
  } else {
    ASSERT(false);
  }
}

FDE_CSSDisplay CFDE_CSSStyleSelector::ToDisplay(FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Block:
      return FDE_CSSDisplay::Block;
    case FDE_CSSPropertyValue::None:
      return FDE_CSSDisplay::None;
    case FDE_CSSPropertyValue::ListItem:
      return FDE_CSSDisplay::ListItem;
    case FDE_CSSPropertyValue::InlineTable:
      return FDE_CSSDisplay::InlineTable;
    case FDE_CSSPropertyValue::InlineBlock:
      return FDE_CSSDisplay::InlineBlock;
    case FDE_CSSPropertyValue::Inline:
    default:
      return FDE_CSSDisplay::Inline;
  }
}

FDE_CSSTextAlign CFDE_CSSStyleSelector::ToTextAlign(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Center:
      return FDE_CSSTextAlign::Center;
    case FDE_CSSPropertyValue::Right:
      return FDE_CSSTextAlign::Right;
    case FDE_CSSPropertyValue::Justify:
      return FDE_CSSTextAlign::Justify;
    case FDE_CSSPropertyValue::Left:
    default:
      return FDE_CSSTextAlign::Left;
  }
}

uint16_t CFDE_CSSStyleSelector::ToFontWeight(FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Bold:
      return 700;
    case FDE_CSSPropertyValue::Bolder:
      return 900;
    case FDE_CSSPropertyValue::Lighter:
      return 200;
    case FDE_CSSPropertyValue::Normal:
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

bool CFDE_CSSStyleSelector::SetLengthWithPercent(FDE_CSSLength& width,
                                                 FDE_CSSPrimitiveType eType,
                                                 CFDE_CSSValue* pValue,
                                                 FX_FLOAT fFontSize) {
  if (eType == FDE_CSSPrimitiveType::Number) {
    const CFDE_CSSNumberValue* v = ToValue<CFDE_CSSNumberValue>(pValue);
    if (v->Kind() == FDE_CSSNumberType::Percent) {
      width.Set(FDE_CSSLengthUnit::Percent,
                ToValue<CFDE_CSSNumberValue>(pValue)->Value() / 100.0f);
      return width.NonZero();
    }

    FX_FLOAT fValue = v->Apply(fFontSize);
    width.Set(FDE_CSSLengthUnit::Point, fValue);
    return width.NonZero();
  } else if (eType == FDE_CSSPrimitiveType::Enum) {
    switch (ToValue<CFDE_CSSEnumValue>(pValue)->Value()) {
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
    case FDE_CSSPropertyValue::Baseline:
    default:
      return FDE_CSSVerticalAlign::Baseline;
  }
}

uint32_t CFDE_CSSStyleSelector::ToTextDecoration(CFDE_CSSValueList* pValue) {
  uint32_t dwDecoration = 0;
  for (int32_t i = pValue->CountValues() - 1; i >= 0; --i) {
    CFDE_CSSValue* pVal = pValue->GetValue(i);
    if (pVal->GetType() != FDE_CSSPrimitiveType::Enum)
      continue;

    switch (ToValue<CFDE_CSSEnumValue>(pVal)->Value()) {
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
  return dwDecoration;
}

FDE_CSSFontVariant CFDE_CSSStyleSelector::ToFontVariant(
    FDE_CSSPropertyValue eValue) {
  return eValue == FDE_CSSPropertyValue::SmallCaps
             ? FDE_CSSFontVariant::SmallCaps
             : FDE_CSSFontVariant::Normal;
}
