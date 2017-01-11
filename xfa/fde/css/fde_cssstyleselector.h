// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_FDE_CSSSTYLESELECTOR_H_
#define XFA_FDE_CSS_FDE_CSSSTYLESELECTOR_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/fx_ext.h"
#include "xfa/fde/css/fde_css.h"
#include "xfa/fde/css/fde_csscache.h"
#include "xfa/fde/css/fde_cssdeclaration.h"

class CFDE_CSSAccelerator;
class CFDE_CSSComputedStyle;
class CXFA_CSSTagProvider;

class FDE_CSSRuleData {
 public:
  FDE_CSSRuleData(CFDE_CSSSelector* pSel,
                  CFDE_CSSDeclaration* pDecl,
                  uint32_t dwPos);

  CFDE_CSSSelector* const pSelector;
  CFDE_CSSDeclaration* const pDeclaration;
  uint32_t dwPriority;
  FDE_CSSRuleData* pNext;
};

class CFDE_CSSRuleCollection {
 public:
  CFDE_CSSRuleCollection();
  ~CFDE_CSSRuleCollection();

  void AddRulesFrom(const CFDE_CSSStyleSheetArray& sheets,
                    uint32_t dwMediaList,
                    CFGAS_FontMgr* pFontMgr);
  void Clear();
  int32_t CountSelectors() const { return m_iSelectors; }

  FDE_CSSRuleData* GetIDRuleData(uint32_t dwIDHash) {
    auto it = m_IDRules.find(dwIDHash);
    return it != m_IDRules.end() ? it->second : nullptr;
  }

  FDE_CSSRuleData* GetTagRuleData(uint32_t dwTagHash) {
    auto it = m_TagRules.find(dwTagHash);
    return it != m_TagRules.end() ? it->second : nullptr;
  }

  FDE_CSSRuleData* GetClassRuleData(uint32_t dwIDHash) {
    auto it = m_ClassRules.find(dwIDHash);
    return it != m_ClassRules.end() ? it->second : nullptr;
  }

  FDE_CSSRuleData* GetUniversalRuleData() { return m_pUniversalRules; }
  FDE_CSSRuleData* GetPseudoRuleData() { return m_pPseudoRules; }

 protected:
  void AddRulesFrom(IFDE_CSSStyleSheet* pStyleSheet,
                    IFDE_CSSRule* pRule,
                    uint32_t dwMediaList,
                    CFGAS_FontMgr* pFontMgr);
  void AddRuleTo(std::map<uint32_t, FDE_CSSRuleData*>* pMap,
                 uint32_t dwKey,
                 CFDE_CSSSelector* pSel,
                 CFDE_CSSDeclaration* pDecl);
  bool AddRuleTo(FDE_CSSRuleData** pList, FDE_CSSRuleData* pData);
  FDE_CSSRuleData* NewRuleData(CFDE_CSSSelector* pSel,
                               CFDE_CSSDeclaration* pDecl);

  std::map<uint32_t, FDE_CSSRuleData*> m_IDRules;
  std::map<uint32_t, FDE_CSSRuleData*> m_TagRules;
  std::map<uint32_t, FDE_CSSRuleData*> m_ClassRules;
  FDE_CSSRuleData* m_pUniversalRules;
  FDE_CSSRuleData* m_pPseudoRules;
  int32_t m_iSelectors;
};

class CFDE_CSSStyleSelector {
 public:
  explicit CFDE_CSSStyleSelector(CFGAS_FontMgr* pFontMgr);
  ~CFDE_CSSStyleSelector();

  void SetDefFontSize(FX_FLOAT fFontSize);

  bool SetStyleSheet(FDE_CSSStyleSheetGroup eType, IFDE_CSSStyleSheet* pSheet);
  bool SetStyleSheets(FDE_CSSStyleSheetGroup eType,
                      const CFDE_CSSStyleSheetArray* pArray);
  void SetStylePriority(FDE_CSSStyleSheetGroup eType,
                        FDE_CSSStyleSheetPriority ePriority);
  void UpdateStyleIndex(uint32_t dwMediaList);
  CFDE_CSSAccelerator* InitAccelerator();
  IFDE_CSSComputedStyle* CreateComputedStyle(
      IFDE_CSSComputedStyle* pParentStyle);
  int32_t MatchDeclarations(CXFA_CSSTagProvider* pTag,
                            CFDE_CSSDeclarationArray& matchedDecls,
                            FDE_CSSPseudo ePseudoType = FDE_CSSPseudo::NONE);
  void ComputeStyle(CXFA_CSSTagProvider* pTag,
                    const CFDE_CSSDeclaration** ppDeclArray,
                    int32_t iDeclCount,
                    IFDE_CSSComputedStyle* pDestStyle);

 protected:
  void Reset();
  void MatchRules(FDE_CSSTagCache* pCache,
                  FDE_CSSRuleData* pList,
                  FDE_CSSPseudo ePseudoType);
  bool MatchSelector(FDE_CSSTagCache* pCache,
                     CFDE_CSSSelector* pSel,
                     FDE_CSSPseudo ePseudoType);
  void AppendInlineStyle(CFDE_CSSDeclaration* pDecl,
                         const FX_WCHAR* psz,
                         int32_t iLen);
  void ApplyDeclarations(bool bPriority,
                         const CFDE_CSSDeclaration** ppDeclArray,
                         int32_t iDeclCount,
                         IFDE_CSSComputedStyle* pDestStyle);
  void ApplyProperty(FDE_CSSProperty eProperty,
                     IFDE_CSSValue* pValue,
                     CFDE_CSSComputedStyle* pComputedStyle);

  FX_FLOAT ApplyNumber(FDE_CSSPrimitiveType eUnit,
                       FX_FLOAT fValue,
                       FX_FLOAT fPercentBase);
  bool SetLengthWithPercent(FDE_CSSLENGTH& width,
                            FDE_CSSPrimitiveType eType,
                            IFDE_CSSPrimitiveValue* pPrimitive,
                            FX_FLOAT fFontSize);
  FX_FLOAT ToFontSize(FDE_CSSPropertyValue eValue, FX_FLOAT fCurFontSize);
  FDE_CSSDisplay ToDisplay(FDE_CSSPropertyValue eValue);
  FDE_CSSTextAlign ToTextAlign(FDE_CSSPropertyValue eValue);
  uint16_t ToFontWeight(FDE_CSSPropertyValue eValue);
  FDE_CSSFontStyle ToFontStyle(FDE_CSSPropertyValue eValue);
  FDE_CSSBorderStyle ToBorderStyle(FDE_CSSPropertyValue eValue);
  FDE_CSSVerticalAlign ToVerticalAlign(FDE_CSSPropertyValue eValue);
  FDE_CSSListStyleType ToListStyleType(FDE_CSSPropertyValue eValue);
  FDE_CSSListStylePosition ToListStylePosition(FDE_CSSPropertyValue eValue);
  FDE_CSSVisibility ToVisibility(FDE_CSSPropertyValue eValue);
  FDE_CSSWhitespace ToWhiteSpace(FDE_CSSPropertyValue eValue);
  uint32_t ToTextDecoration(IFDE_CSSValueList* pList);
  FDE_CSSTextTransform ToTextTransform(FDE_CSSPropertyValue eValue);
  FDE_CSSFontVariant ToFontVariant(FDE_CSSPropertyValue eValue);
  FDE_CSSFloat ToFloat(FDE_CSSPropertyValue eValue);
  FDE_CSSClear ToClear(FDE_CSSPropertyValue eValue);
  FDE_CSSWritingMode ToWritingMode(FDE_CSSPropertyValue eValue);
  FDE_CSSWordBreak ToWordBreak(FDE_CSSPropertyValue eValue);
  FDE_CSSPageBreak ToPageBreak(FDE_CSSPropertyValue eValue);
  FDE_CSSOverFlow ToOverflow(FDE_CSSPropertyValue eValue);
  FDE_CSSLineBreak ToLineBreak(FDE_CSSPropertyValue eValue);
  FDE_CSSTextCombine ToTextCombine(FDE_CSSPropertyValue eValue);
  bool ToTextEmphasisMark(FDE_CSSPropertyValue eValue,
                          FDE_CSSTextEmphasisMark& eMark);
  bool ToTextEmphasisFill(FDE_CSSPropertyValue eValue,
                          FDE_CSSTextEmphasisFill& eFill);
  FDE_CSSCursor ToCursor(FDE_CSSPropertyValue eValue);
  FDE_CSSPosition ToPosition(FDE_CSSPropertyValue eValue);
  FDE_CSSCaptionSide ToCaptionSide(FDE_CSSPropertyValue eValue);
  FDE_CSSBackgroundRepeat ToBKGRepeat(FDE_CSSPropertyValue eValue);
  FDE_CSSBackgroundAttachment ToBKGAttachment(FDE_CSSPropertyValue eValue);
  FDE_CSSRubyAlign ToRubyAlign(FDE_CSSPropertyValue eValue);
  FDE_CSSRubyOverhang ToRubyOverhang(FDE_CSSPropertyValue eValue);
  FDE_CSSRubyPosition ToRubyPosition(FDE_CSSPropertyValue eValue);
  FDE_CSSRubySpan ToRubySpan(FDE_CSSPropertyValue eValue);

  CFGAS_FontMgr* const m_pFontMgr;
  FX_FLOAT m_fDefFontSize;
  CFDE_CSSStyleSheetArray m_SheetGroups[3];
  CFDE_CSSRuleCollection m_RuleCollection[3];
  FDE_CSSStyleSheetGroup m_ePriorities[3];
  std::unique_ptr<CFDE_CSSAccelerator> m_pAccelerator;
  std::vector<FDE_CSSRuleData*> m_MatchedRules;
};

struct FDE_CSSCOUNTERDATA {
 public:
  FDE_CSSCOUNTERDATA() { FXSYS_memset(this, 0, sizeof(FDE_CSSCOUNTERDATA)); }
  bool GetCounterIncrement(int32_t& iValue) {
    iValue = m_iIncVal;
    return m_bIncrement;
  }
  bool GetCounterReset(int32_t& iValue) {
    iValue = m_iResetVal;
    return m_bReset;
  }

  const FX_WCHAR* m_pszIdent;
  bool m_bIncrement;
  bool m_bReset;
  int32_t m_iIncVal;
  int32_t m_iResetVal;
};

class CFDE_CSSCounterStyle {
 public:
  CFDE_CSSCounterStyle();
  ~CFDE_CSSCounterStyle();

  void SetCounterIncrementList(IFDE_CSSValueList* pList) {
    m_pCounterInc = pList;
    m_bIndexDirty = true;
  }
  void SetCounterResetList(IFDE_CSSValueList* pList) {
    m_pCounterReset = pList;
    m_bIndexDirty = true;
  }
  int32_t CountCounters() {
    UpdateIndex();
    return m_arrCounterData.GetSize();
  }
  bool GetCounterIncrement(int32_t index, int32_t& iValue) {
    UpdateIndex();
    return m_arrCounterData.ElementAt(index).GetCounterIncrement(iValue);
  }
  bool GetCounterReset(int32_t index, int32_t& iValue) {
    UpdateIndex();
    return m_arrCounterData.ElementAt(index).GetCounterReset(iValue);
  }
  const FX_WCHAR* GetCounterIdentifier(int32_t index) {
    UpdateIndex();
    return m_arrCounterData.ElementAt(index).m_pszIdent;
  }

 protected:
  void UpdateIndex();
  void DoUpdateIndex(IFDE_CSSValueList* pList);
  int32_t FindIndex(const FX_WCHAR* pszIdentifier);

  IFDE_CSSValueList* m_pCounterInc;
  IFDE_CSSValueList* m_pCounterReset;
  CFX_ArrayTemplate<FDE_CSSCOUNTERDATA> m_arrCounterData;
  bool m_bIndexDirty;
};

class CFDE_CSSInheritedData {
 public:
  CFDE_CSSInheritedData();

  void Reset();

  const FX_WCHAR* m_pszListStyleImage;
  FDE_CSSLENGTH m_LetterSpacing;
  FDE_CSSLENGTH m_WordSpacing;
  FDE_CSSLENGTH m_TextIndent;
  IFDE_CSSValueList* m_pFontFamily;
  IFDE_CSSValueList* m_pQuotes;
  IFDE_CSSValueList* m_pCursorUris;
  FDE_CSSCursor m_eCursor;
  FX_FLOAT m_fFontSize;
  FX_FLOAT m_fLineHeight;
  FX_ARGB m_dwFontColor;
  FX_ARGB m_dwTextEmphasisColor;
  uint16_t m_wFontWeight;
  int32_t m_iWidows;
  int32_t m_iOrphans;
  const FX_WCHAR* m_pszTextEmphasisCustomMark;
  FDE_CSSFontVariant m_eFontVariant;
  FDE_CSSFontStyle m_eFontStyle;
  uint32_t m_bTextEmphasisColorCurrent : 1;
  FDE_CSSTextAlign m_eTextAlign;
  FDE_CSSVisibility m_eVisibility;
  FDE_CSSWhitespace m_eWhiteSpace;
  FDE_CSSTextTransform m_eTextTransform;
  FDE_CSSWritingMode m_eWritingMode;
  FDE_CSSWordBreak m_eWordBreak;
  FDE_CSSLineBreak m_eLineBreak;
  FDE_CSSTextEmphasisFill m_eTextEmphasisFill;
  FDE_CSSTextEmphasisMark m_eTextEmphasisMark;
  FDE_CSSCaptionSide m_eCaptionSide;
  FDE_CSSRubyAlign m_eRubyAlign;
  FDE_CSSRubyOverhang m_eRubyOverhang;
  FDE_CSSRubyPosition m_eRubyPosition;
};

class CFDE_CSSNonInheritedData {
 public:
  CFDE_CSSNonInheritedData();

  void Reset();

  IFDE_CSSValueList* m_pContentList;
  CFDE_CSSCounterStyle* m_pCounterStyle;
  FDE_CSSRECT m_MarginWidth;
  FDE_CSSRECT m_BorderWidth;
  FDE_CSSRECT m_PaddingWidth;
  FDE_CSSSIZE m_BoxSize;
  FDE_CSSSIZE m_MinBoxSize;
  FDE_CSSSIZE m_MaxBoxSize;
  FDE_CSSPOINT m_BKGPosition;
  const FX_WCHAR* m_pszBKGImage;
  FX_ARGB m_dwBKGColor;
  FX_ARGB m_dwBDRLeftColor;
  FX_ARGB m_dwBDRTopColor;
  FX_ARGB m_dwBDRRightColor;
  FX_ARGB m_dwBDRBottomColor;
  IFDE_CSSValue* m_pRubySpan;
  FDE_CSSLENGTH m_ColumnCount;
  FDE_CSSLENGTH m_ColumnGap;
  FDE_CSSLENGTH m_ColumnRuleWidth;
  FDE_CSSLENGTH m_ColumnWidth;
  FX_ARGB m_dwColumnRuleColor;
  FDE_CSSLENGTH m_Top;
  FDE_CSSLENGTH m_Bottom;
  FDE_CSSLENGTH m_Left;
  FDE_CSSLENGTH m_Right;

  FX_FLOAT m_fVerticalAlign;
  FX_FLOAT m_fTextCombineNumber;
  FDE_CSSBorderStyle m_eBDRLeftStyle;
  FDE_CSSBorderStyle m_eBDRTopStyle;
  FDE_CSSBorderStyle m_eBDRRightStyle;
  FDE_CSSBorderStyle m_eBDRBottomStyle;
  FDE_CSSDisplay m_eDisplay;
  FDE_CSSVerticalAlign m_eVerticalAlign;
  FDE_CSSListStyleType m_eListStyleType;
  FDE_CSSBorderStyle m_eColumnRuleStyle;
  FDE_CSSPageBreak m_ePageBreakInside;
  FDE_CSSPageBreak m_ePageBreakAfter;
  FDE_CSSPageBreak m_ePageBreakBefore;
  FDE_CSSPosition m_ePosition;
  FDE_CSSBackgroundRepeat m_eBKGRepeat;
  FDE_CSSFloat m_eFloat;
  FDE_CSSClear m_eClear;
  FDE_CSSOverFlow m_eOverflowX;
  FDE_CSSOverFlow m_eOverflowY;
  FDE_CSSListStylePosition m_eListStylePosition;
  FDE_CSSBackgroundAttachment m_eBKGAttachment;
  uint32_t m_bHasMargin : 1;
  uint32_t m_bHasBorder : 1;
  uint32_t m_bHasPadding : 1;
  uint32_t m_dwTextDecoration : 5;
  FDE_CSSTextCombine m_eTextCombine;
  uint32_t m_bColumnRuleColorSame : 1;
  uint32_t m_bHasTextCombineNumber : 1;
};

class CFDE_CSSComputedStyle : public IFDE_CSSComputedStyle,
                              public IFDE_CSSBoundaryStyle,
                              public IFDE_CSSFontStyle,
                              public IFDE_CSSPositionStyle,
                              public IFDE_CSSParagraphStyle {
 public:
  CFDE_CSSComputedStyle();
  ~CFDE_CSSComputedStyle() override;

  // IFX_Retainable
  uint32_t Retain() override;
  uint32_t Release() override;

  // IFDE_CSSComputedStyle
  void Reset() override;
  IFDE_CSSFontStyle* GetFontStyles() override;
  IFDE_CSSBoundaryStyle* GetBoundaryStyles() override;
  IFDE_CSSPositionStyle* GetPositionStyles() override;
  IFDE_CSSParagraphStyle* GetParagraphStyles() override;
  bool GetCustomStyle(const CFX_WideStringC& wsName,
                      CFX_WideString& wsValue) const override;

  // IFDE_CSSFontStyle:
  int32_t CountFontFamilies() const override;
  const FX_WCHAR* GetFontFamily(int32_t index) const override;
  uint16_t GetFontWeight() const override;
  FDE_CSSFontVariant GetFontVariant() const override;
  FDE_CSSFontStyle GetFontStyle() const override;
  FX_FLOAT GetFontSize() const override;
  FX_ARGB GetColor() const override;
  void SetFontWeight(uint16_t wFontWeight) override;
  void SetFontVariant(FDE_CSSFontVariant eFontVariant) override;
  void SetFontStyle(FDE_CSSFontStyle eFontStyle) override;
  void SetFontSize(FX_FLOAT fFontSize) override;
  void SetColor(FX_ARGB dwFontColor) override;

  // IFDE_CSSBoundaryStyle:
  const FDE_CSSRECT* GetBorderWidth() const override;
  const FDE_CSSRECT* GetMarginWidth() const override;
  const FDE_CSSRECT* GetPaddingWidth() const override;
  void SetMarginWidth(const FDE_CSSRECT& rect) override;
  void SetPaddingWidth(const FDE_CSSRECT& rect) override;

  // IFDE_CSSPositionStyle:
  FDE_CSSDisplay GetDisplay() const override;

  // IFDE_CSSParagraphStyle:
  FX_FLOAT GetLineHeight() const override;
  const FDE_CSSLENGTH& GetTextIndent() const override;
  FDE_CSSTextAlign GetTextAlign() const override;
  FDE_CSSVerticalAlign GetVerticalAlign() const override;
  FX_FLOAT GetNumberVerticalAlign() const override;
  uint32_t GetTextDecoration() const override;
  const FDE_CSSLENGTH& GetLetterSpacing() const override;
  void SetLineHeight(FX_FLOAT fLineHeight) override;
  void SetTextIndent(const FDE_CSSLENGTH& textIndent) override;
  void SetTextAlign(FDE_CSSTextAlign eTextAlign) override;
  void SetNumberVerticalAlign(FX_FLOAT fAlign) override;
  void SetTextDecoration(uint32_t dwTextDecoration) override;
  void SetLetterSpacing(const FDE_CSSLENGTH& letterSpacing) override;
  void AddCustomStyle(const CFX_WideString& wsName,
                      const CFX_WideString& wsValue);

  uint32_t m_dwRefCount;
  CFDE_CSSInheritedData m_InheritedData;
  CFDE_CSSNonInheritedData m_NonInheritedData;
  std::vector<CFX_WideString> m_CustomProperties;
};

#endif  // XFA_FDE_CSS_FDE_CSSSTYLESELECTOR_H_
