// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/fde_cssdeclaration.h"

#include "core/fxcrt/fx_ext.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fde/css/cfde_csscolorvalue.h"
#include "xfa/fde/css/cfde_cssenumvalue.h"
#include "xfa/fde/css/cfde_cssfunction.h"
#include "xfa/fde/css/cfde_cssfunctionvalue.h"
#include "xfa/fde/css/cfde_cssnumbervalue.h"
#include "xfa/fde/css/cfde_cssstringvalue.h"
#include "xfa/fde/css/cfde_cssvaluelist.h"
#include "xfa/fde/css/cfde_cssvaluelistparser.h"

CFDE_CSSDeclaration::CFDE_CSSDeclaration() {}

CFDE_CSSDeclaration::~CFDE_CSSDeclaration() {}

CFDE_CSSValue* CFDE_CSSDeclaration::GetProperty(FDE_CSSProperty eProperty,
                                                bool& bImportant) const {
  for (const auto& p : properties_) {
    if (p->eProperty == eProperty) {
      bImportant = p->bImportant;
      return p->pValue.Get();
    }
  }
  return nullptr;
}

const FX_WCHAR* CFDE_CSSDeclaration::CopyToLocal(
    const FDE_CSSPropertyArgs* pArgs,
    const FX_WCHAR* pszValue,
    int32_t iValueLen) {
  ASSERT(iValueLen > 0);
  std::unordered_map<uint32_t, FX_WCHAR*>* pCache = pArgs->pStringCache;
  uint32_t key = 0;
  if (pCache) {
    key = FX_HashCode_GetW(CFX_WideStringC(pszValue, iValueLen), false);
    auto it = pCache->find(key);
    if (it != pCache->end())
      return it->second;
  }
  FX_WCHAR* psz = FX_Alloc(FX_WCHAR, iValueLen + 1);
  FXSYS_wcsncpy(psz, pszValue, iValueLen);
  psz[iValueLen] = '\0';
  if (pCache)
    (*pCache)[key] = psz;

  return psz;
}

void CFDE_CSSDeclaration::AddPropertyHolder(FDE_CSSProperty eProperty,
                                            CFX_RetainPtr<CFDE_CSSValue> pValue,
                                            bool bImportant) {
  auto pHolder = pdfium::MakeUnique<FDE_CSSPropertyHolder>();
  pHolder->bImportant = bImportant;
  pHolder->eProperty = eProperty;
  pHolder->pValue = pValue;
  properties_.push_back(std::move(pHolder));
}

void CFDE_CSSDeclaration::AddProperty(const FDE_CSSPropertyArgs* pArgs,
                                      const FX_WCHAR* pszValue,
                                      int32_t iValueLen) {
  ASSERT(iValueLen > 0);
  bool bImportant = false;
  if (iValueLen >= 10 && pszValue[iValueLen - 10] == '!' &&
      FXSYS_wcsnicmp(L"important", pszValue + iValueLen - 9, 9) == 0) {
    if ((iValueLen -= 10) == 0)
      return;

    bImportant = true;
  }
  const uint32_t dwType = pArgs->pProperty->dwType;
  switch (dwType & 0x0F) {
    case FDE_CSSVALUETYPE_Primitive: {
      static const uint32_t g_ValueGuessOrder[] = {
          FDE_CSSVALUETYPE_MaybeNumber, FDE_CSSVALUETYPE_MaybeEnum,
          FDE_CSSVALUETYPE_MaybeColor,  FDE_CSSVALUETYPE_MaybeFunction,
          FDE_CSSVALUETYPE_MaybeString,
      };
      static const int32_t g_ValueGuessCount =
          sizeof(g_ValueGuessOrder) / sizeof(uint32_t);
      for (int32_t i = 0; i < g_ValueGuessCount; ++i) {
        const uint32_t dwMatch = dwType & g_ValueGuessOrder[i];
        if (dwMatch == 0) {
          continue;
        }
        CFX_RetainPtr<CFDE_CSSValue> pCSSValue;
        switch (dwMatch) {
          case FDE_CSSVALUETYPE_MaybeFunction:
            pCSSValue = ParseFunction(pArgs, pszValue, iValueLen);
            break;
          case FDE_CSSVALUETYPE_MaybeNumber:
            pCSSValue = ParseNumber(pArgs, pszValue, iValueLen);
            break;
          case FDE_CSSVALUETYPE_MaybeEnum:
            pCSSValue = ParseEnum(pArgs, pszValue, iValueLen);
            break;
          case FDE_CSSVALUETYPE_MaybeColor:
            pCSSValue = ParseColor(pArgs, pszValue, iValueLen);
            break;
          case FDE_CSSVALUETYPE_MaybeString:
            pCSSValue = ParseString(pArgs, pszValue, iValueLen);
            break;
          default:
            break;
        }
        if (pCSSValue) {
          AddPropertyHolder(pArgs->pProperty->eName, pCSSValue, bImportant);
          return;
        }
        if (FDE_IsOnlyValue(dwType, g_ValueGuessOrder[i]))
          return;
      }
      break;
    }
    case FDE_CSSVALUETYPE_Shorthand: {
      CFX_RetainPtr<CFDE_CSSValue> pWidth;
      switch (pArgs->pProperty->eName) {
        case FDE_CSSProperty::Font:
          ParseFontProperty(pArgs, pszValue, iValueLen, bImportant);
          return;
        case FDE_CSSProperty::Border:
          if (ParseBorderProperty(pszValue, iValueLen, pWidth)) {
            AddPropertyHolder(FDE_CSSProperty::BorderLeftWidth, pWidth,
                              bImportant);
            AddPropertyHolder(FDE_CSSProperty::BorderTopWidth, pWidth,
                              bImportant);
            AddPropertyHolder(FDE_CSSProperty::BorderRightWidth, pWidth,
                              bImportant);
            AddPropertyHolder(FDE_CSSProperty::BorderBottomWidth, pWidth,
                              bImportant);
            return;
          }
          break;
        case FDE_CSSProperty::BorderLeft:
          if (ParseBorderProperty(pszValue, iValueLen, pWidth)) {
            AddPropertyHolder(FDE_CSSProperty::BorderLeftWidth, pWidth,
                              bImportant);
            return;
          }
          break;
        case FDE_CSSProperty::BorderTop:
          if (ParseBorderProperty(pszValue, iValueLen, pWidth)) {
            AddPropertyHolder(FDE_CSSProperty::BorderTopWidth, pWidth,
                              bImportant);
            return;
          }
          break;
        case FDE_CSSProperty::BorderRight:
          if (ParseBorderProperty(pszValue, iValueLen, pWidth)) {
            AddPropertyHolder(FDE_CSSProperty::BorderRightWidth, pWidth,
                              bImportant);
            return;
          }
          break;
        case FDE_CSSProperty::BorderBottom:
          if (ParseBorderProperty(pszValue, iValueLen, pWidth)) {
            AddPropertyHolder(FDE_CSSProperty::BorderBottomWidth, pWidth,
                              bImportant);
            return;
          }
          break;
        default:
          break;
      }
    } break;
    case FDE_CSSVALUETYPE_List:
      ParseValueListProperty(pArgs, pszValue, iValueLen, bImportant);
      return;
    default:
      ASSERT(false);
      break;
  }
}

void CFDE_CSSDeclaration::AddProperty(const FDE_CSSPropertyArgs* pArgs,
                                      const FX_WCHAR* pszName,
                                      int32_t iNameLen,
                                      const FX_WCHAR* pszValue,
                                      int32_t iValueLen) {
  auto pProperty = pdfium::MakeUnique<FDE_CSSCustomProperty>();
  pProperty->pwsName = CopyToLocal(pArgs, pszName, iNameLen);
  pProperty->pwsValue = CopyToLocal(pArgs, pszValue, iValueLen);
  custom_properties_.push_back(std::move(pProperty));
}

CFX_RetainPtr<CFDE_CSSValue> CFDE_CSSDeclaration::ParseNumber(
    const FDE_CSSPropertyArgs* pArgs,
    const FX_WCHAR* pszValue,
    int32_t iValueLen) {
  FX_FLOAT fValue;
  FDE_CSSNumberType eUnit;
  if (!FDE_ParseCSSNumber(pszValue, iValueLen, fValue, eUnit))
    return nullptr;
  return pdfium::MakeRetain<CFDE_CSSNumberValue>(eUnit, fValue);
}

CFX_RetainPtr<CFDE_CSSValue> CFDE_CSSDeclaration::ParseEnum(
    const FDE_CSSPropertyArgs* pArgs,
    const FX_WCHAR* pszValue,
    int32_t iValueLen) {
  const FDE_CSSPropertyValueTable* pValue =
      FDE_GetCSSPropertyValueByName(CFX_WideStringC(pszValue, iValueLen));
  return pValue ? pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName)
                : nullptr;
}

CFX_RetainPtr<CFDE_CSSValue> CFDE_CSSDeclaration::ParseColor(
    const FDE_CSSPropertyArgs* pArgs,
    const FX_WCHAR* pszValue,
    int32_t iValueLen) {
  FX_ARGB dwColor;
  if (!FDE_ParseCSSColor(pszValue, iValueLen, dwColor))
    return nullptr;
  return pdfium::MakeRetain<CFDE_CSSColorValue>(dwColor);
}

CFX_RetainPtr<CFDE_CSSValue> CFDE_CSSDeclaration::ParseString(
    const FDE_CSSPropertyArgs* pArgs,
    const FX_WCHAR* pszValue,
    int32_t iValueLen) {
  int32_t iOffset;
  if (!FDE_ParseCSSString(pszValue, iValueLen, &iOffset, &iValueLen))
    return nullptr;

  if (iValueLen <= 0)
    return nullptr;

  pszValue = CopyToLocal(pArgs, pszValue + iOffset, iValueLen);
  return pszValue ? pdfium::MakeRetain<CFDE_CSSStringValue>(pszValue) : nullptr;
}

CFX_RetainPtr<CFDE_CSSValue> CFDE_CSSDeclaration::ParseFunction(
    const FDE_CSSPropertyArgs* pArgs,
    const FX_WCHAR* pszValue,
    int32_t iValueLen) {
  if (pszValue[iValueLen - 1] != ')')
    return nullptr;

  int32_t iStartBracket = 0;
  while (pszValue[iStartBracket] != '(') {
    if (iStartBracket >= iValueLen)
      return nullptr;
    iStartBracket++;
  }
  if (iStartBracket == 0)
    return nullptr;

  const FX_WCHAR* pszFuncName = CopyToLocal(pArgs, pszValue, iStartBracket);
  pszValue += (iStartBracket + 1);
  iValueLen -= (iStartBracket + 2);
  std::vector<CFX_RetainPtr<CFDE_CSSValue>> argumentArr;
  CFDE_CSSValueListParser parser(pszValue, iValueLen, ',');
  FDE_CSSPrimitiveType ePrimitiveType;
  while (parser.NextValue(ePrimitiveType, pszValue, iValueLen)) {
    switch (ePrimitiveType) {
      case FDE_CSSPrimitiveType::String: {
        const FDE_CSSPropertyValueTable* pPropertyValue =
            FDE_GetCSSPropertyValueByName(CFX_WideStringC(pszValue, iValueLen));
        if (pPropertyValue) {
          argumentArr.push_back(
              pdfium::MakeRetain<CFDE_CSSEnumValue>(pPropertyValue->eName));
          continue;
        }

        auto pFunctionValue = ParseFunction(pArgs, pszValue, iValueLen);
        if (pFunctionValue) {
          argumentArr.push_back(pFunctionValue);
          continue;
        }
        argumentArr.push_back(pdfium::MakeRetain<CFDE_CSSStringValue>(
            CopyToLocal(pArgs, pszValue, iValueLen)));
        break;
      }
      case FDE_CSSPrimitiveType::Number: {
        FX_FLOAT fValue;
        FDE_CSSNumberType eNumberType;
        if (FDE_ParseCSSNumber(pszValue, iValueLen, fValue, eNumberType))
          argumentArr.push_back(
              pdfium::MakeRetain<CFDE_CSSNumberValue>(eNumberType, fValue));
        break;
      }
      default:
        argumentArr.push_back(pdfium::MakeRetain<CFDE_CSSStringValue>(
            CopyToLocal(pArgs, pszValue, iValueLen)));
        break;
    }
  }

  auto pArgumentList = pdfium::MakeRetain<CFDE_CSSValueList>(argumentArr);
  auto pFunction =
      pdfium::MakeUnique<CFDE_CSSFunction>(pszFuncName, pArgumentList);
  return pdfium::MakeRetain<CFDE_CSSFunctionValue>(std::move(pFunction));
}

void CFDE_CSSDeclaration::ParseValueListProperty(
    const FDE_CSSPropertyArgs* pArgs,
    const FX_WCHAR* pszValue,
    int32_t iValueLen,
    bool bImportant) {
  FX_WCHAR separator =
      (pArgs->pProperty->eName == FDE_CSSProperty::FontFamily) ? ',' : ' ';
  CFDE_CSSValueListParser parser(pszValue, iValueLen, separator);

  const uint32_t dwType = pArgs->pProperty->dwType;
  FDE_CSSPrimitiveType eType;
  std::vector<CFX_RetainPtr<CFDE_CSSValue>> list;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPrimitiveType::Number:
        if (dwType & FDE_CSSVALUETYPE_MaybeNumber) {
          FX_FLOAT fValue;
          FDE_CSSNumberType eNumType;
          if (FDE_ParseCSSNumber(pszValue, iValueLen, fValue, eNumType))
            list.push_back(
                pdfium::MakeRetain<CFDE_CSSNumberValue>(eNumType, fValue));
        }
        break;
      case FDE_CSSPrimitiveType::String:
        if (dwType & FDE_CSSVALUETYPE_MaybeColor) {
          FX_ARGB dwColor;
          if (FDE_ParseCSSColor(pszValue, iValueLen, dwColor)) {
            list.push_back(pdfium::MakeRetain<CFDE_CSSColorValue>(dwColor));
            continue;
          }
        }
        if (dwType & FDE_CSSVALUETYPE_MaybeEnum) {
          const FDE_CSSPropertyValueTable* pValue =
              FDE_GetCSSPropertyValueByName(
                  CFX_WideStringC(pszValue, iValueLen));
          if (pValue) {
            list.push_back(
                pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName));
            continue;
          }
        }
        if (dwType & FDE_CSSVALUETYPE_MaybeString) {
          pszValue = CopyToLocal(pArgs, pszValue, iValueLen);
          list.push_back(pdfium::MakeRetain<CFDE_CSSStringValue>(pszValue));
        }
        break;
      case FDE_CSSPrimitiveType::RGB:
        if (dwType & FDE_CSSVALUETYPE_MaybeColor) {
          FX_ARGB dwColor;
          if (FDE_ParseCSSColor(pszValue, iValueLen, dwColor)) {
            list.push_back(pdfium::MakeRetain<CFDE_CSSColorValue>(dwColor));
          }
        }
        break;
      default:
        break;
    }
  }
  if (list.empty())
    return;

  switch (pArgs->pProperty->eName) {
    case FDE_CSSProperty::BorderWidth:
      Add4ValuesProperty(list, bImportant, FDE_CSSProperty::BorderLeftWidth,
                         FDE_CSSProperty::BorderTopWidth,
                         FDE_CSSProperty::BorderRightWidth,
                         FDE_CSSProperty::BorderBottomWidth);
      return;
    case FDE_CSSProperty::Margin:
      Add4ValuesProperty(list, bImportant, FDE_CSSProperty::MarginLeft,
                         FDE_CSSProperty::MarginTop,
                         FDE_CSSProperty::MarginRight,
                         FDE_CSSProperty::MarginBottom);
      return;
    case FDE_CSSProperty::Padding:
      Add4ValuesProperty(list, bImportant, FDE_CSSProperty::PaddingLeft,
                         FDE_CSSProperty::PaddingTop,
                         FDE_CSSProperty::PaddingRight,
                         FDE_CSSProperty::PaddingBottom);
      return;
    default: {
      auto pList = pdfium::MakeRetain<CFDE_CSSValueList>(list);
      AddPropertyHolder(pArgs->pProperty->eName, pList, bImportant);
      return;
    }
  }
}

void CFDE_CSSDeclaration::Add4ValuesProperty(
    const std::vector<CFX_RetainPtr<CFDE_CSSValue>>& list,
    bool bImportant,
    FDE_CSSProperty eLeft,
    FDE_CSSProperty eTop,
    FDE_CSSProperty eRight,
    FDE_CSSProperty eBottom) {
  switch (list.size()) {
    case 1:
      AddPropertyHolder(eLeft, list[0], bImportant);
      AddPropertyHolder(eTop, list[0], bImportant);
      AddPropertyHolder(eRight, list[0], bImportant);
      AddPropertyHolder(eBottom, list[0], bImportant);
      return;
    case 2:
      AddPropertyHolder(eLeft, list[1], bImportant);
      AddPropertyHolder(eTop, list[0], bImportant);
      AddPropertyHolder(eRight, list[1], bImportant);
      AddPropertyHolder(eBottom, list[0], bImportant);
      return;
    case 3:
      AddPropertyHolder(eLeft, list[1], bImportant);
      AddPropertyHolder(eTop, list[0], bImportant);
      AddPropertyHolder(eRight, list[1], bImportant);
      AddPropertyHolder(eBottom, list[2], bImportant);
      return;
    case 4:
      AddPropertyHolder(eLeft, list[3], bImportant);
      AddPropertyHolder(eTop, list[0], bImportant);
      AddPropertyHolder(eRight, list[1], bImportant);
      AddPropertyHolder(eBottom, list[2], bImportant);
      return;
    default:
      break;
  }
}

bool CFDE_CSSDeclaration::ParseBorderProperty(
    const FX_WCHAR* pszValue,
    int32_t iValueLen,
    CFX_RetainPtr<CFDE_CSSValue>& pWidth) const {
  pWidth.Reset(nullptr);

  CFDE_CSSValueListParser parser(pszValue, iValueLen, ' ');
  FDE_CSSPrimitiveType eType;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPrimitiveType::Number: {
        if (pWidth)
          continue;

        FX_FLOAT fValue;
        FDE_CSSNumberType eNumType;
        if (FDE_ParseCSSNumber(pszValue, iValueLen, fValue, eNumType))
          pWidth = pdfium::MakeRetain<CFDE_CSSNumberValue>(eNumType, fValue);
        break;
      }
      case FDE_CSSPrimitiveType::String: {
        const FDE_CSSCOLORTABLE* pColorItem =
            FDE_GetCSSColorByName(CFX_WideStringC(pszValue, iValueLen));
        if (pColorItem)
          continue;

        const FDE_CSSPropertyValueTable* pValue =
            FDE_GetCSSPropertyValueByName(CFX_WideStringC(pszValue, iValueLen));
        if (!pValue)
          continue;

        switch (pValue->eName) {
          case FDE_CSSPropertyValue::Thin:
          case FDE_CSSPropertyValue::Thick:
          case FDE_CSSPropertyValue::Medium:
            if (!pWidth)
              pWidth = pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
            break;
          default:
            break;
        }
        break;
      }
      default:
        break;
    }
  }
  if (!pWidth)
    pWidth = pdfium::MakeRetain<CFDE_CSSNumberValue>(FDE_CSSNumberType::Number,
                                                     0.0f);

  return true;
}

void CFDE_CSSDeclaration::ParseFontProperty(const FDE_CSSPropertyArgs* pArgs,
                                            const FX_WCHAR* pszValue,
                                            int32_t iValueLen,
                                            bool bImportant) {
  CFDE_CSSValueListParser parser(pszValue, iValueLen, '/');
  CFX_RetainPtr<CFDE_CSSValue> pStyle;
  CFX_RetainPtr<CFDE_CSSValue> pVariant;
  CFX_RetainPtr<CFDE_CSSValue> pWeight;
  CFX_RetainPtr<CFDE_CSSValue> pFontSize;
  CFX_RetainPtr<CFDE_CSSValue> pLineHeight;
  std::vector<CFX_RetainPtr<CFDE_CSSValue>> familyList;
  FDE_CSSPrimitiveType eType;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPrimitiveType::String: {
        const FDE_CSSPropertyValueTable* pValue =
            FDE_GetCSSPropertyValueByName(CFX_WideStringC(pszValue, iValueLen));
        if (pValue) {
          switch (pValue->eName) {
            case FDE_CSSPropertyValue::XxSmall:
            case FDE_CSSPropertyValue::XSmall:
            case FDE_CSSPropertyValue::Small:
            case FDE_CSSPropertyValue::Medium:
            case FDE_CSSPropertyValue::Large:
            case FDE_CSSPropertyValue::XLarge:
            case FDE_CSSPropertyValue::XxLarge:
            case FDE_CSSPropertyValue::Smaller:
            case FDE_CSSPropertyValue::Larger:
              if (!pFontSize)
                pFontSize =
                    pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              continue;
            case FDE_CSSPropertyValue::Bold:
            case FDE_CSSPropertyValue::Bolder:
            case FDE_CSSPropertyValue::Lighter:
              if (!pWeight)
                pWeight = pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              continue;
            case FDE_CSSPropertyValue::Italic:
            case FDE_CSSPropertyValue::Oblique:
              if (!pStyle)
                pStyle = pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              continue;
            case FDE_CSSPropertyValue::SmallCaps:
              if (!pVariant)
                pVariant = pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              continue;
            case FDE_CSSPropertyValue::Normal:
              if (!pStyle)
                pStyle = pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              else if (!pVariant)
                pVariant = pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              else if (!pWeight)
                pWeight = pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              else if (!pFontSize)
                pFontSize =
                    pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              else if (!pLineHeight)
                pLineHeight =
                    pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              continue;
            default:
              break;
          }
        }
        if (pFontSize) {
          familyList.push_back(pdfium::MakeRetain<CFDE_CSSStringValue>(
              CopyToLocal(pArgs, pszValue, iValueLen)));
        }
        parser.m_Separator = ',';
        break;
      }
      case FDE_CSSPrimitiveType::Number: {
        FX_FLOAT fValue;
        FDE_CSSNumberType eNumType;
        if (!FDE_ParseCSSNumber(pszValue, iValueLen, fValue, eNumType))
          break;
        if (eType == FDE_CSSPrimitiveType::Number) {
          switch ((int32_t)fValue) {
            case 100:
            case 200:
            case 300:
            case 400:
            case 500:
            case 600:
            case 700:
            case 800:
            case 900:
              if (!pWeight)
                pWeight = pdfium::MakeRetain<CFDE_CSSNumberValue>(
                    FDE_CSSNumberType::Number, fValue);
              continue;
          }
        }
        if (!pFontSize)
          pFontSize = pdfium::MakeRetain<CFDE_CSSNumberValue>(eNumType, fValue);
        else if (!pLineHeight)
          pLineHeight =
              pdfium::MakeRetain<CFDE_CSSNumberValue>(eNumType, fValue);
        break;
      }
      default:
        break;
    }
  }

  if (!pStyle)
    pStyle =
        pdfium::MakeRetain<CFDE_CSSEnumValue>(FDE_CSSPropertyValue::Normal);
  if (!pVariant)
    pVariant =
        pdfium::MakeRetain<CFDE_CSSEnumValue>(FDE_CSSPropertyValue::Normal);
  if (!pWeight)
    pWeight =
        pdfium::MakeRetain<CFDE_CSSEnumValue>(FDE_CSSPropertyValue::Normal);
  if (!pFontSize)
    pFontSize =
        pdfium::MakeRetain<CFDE_CSSEnumValue>(FDE_CSSPropertyValue::Medium);
  if (!pLineHeight)
    pLineHeight =
        pdfium::MakeRetain<CFDE_CSSEnumValue>(FDE_CSSPropertyValue::Normal);

  AddPropertyHolder(FDE_CSSProperty::FontStyle, pStyle, bImportant);
  AddPropertyHolder(FDE_CSSProperty::FontVariant, pVariant, bImportant);
  AddPropertyHolder(FDE_CSSProperty::FontWeight, pWeight, bImportant);
  AddPropertyHolder(FDE_CSSProperty::FontSize, pFontSize, bImportant);
  AddPropertyHolder(FDE_CSSProperty::LineHeight, pLineHeight, bImportant);
  if (!familyList.empty()) {
    auto pList = pdfium::MakeRetain<CFDE_CSSValueList>(familyList);
    AddPropertyHolder(FDE_CSSProperty::FontFamily, pList, bImportant);
  }
}

size_t CFDE_CSSDeclaration::PropertyCountForTesting() const {
  return properties_.size();
}

FDE_CSSPropertyHolder::FDE_CSSPropertyHolder() {}

FDE_CSSPropertyHolder::~FDE_CSSPropertyHolder() {}
