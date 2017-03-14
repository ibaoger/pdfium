// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_FGAS_RTFBREAK_H_
#define XFA_FGAS_LAYOUT_FGAS_RTFBREAK_H_

#include <deque>
#include <vector>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_ucd.h"
#include "xfa/fgas/layout/fgas_textbreak.h"

class CFGAS_GEFont;

#define FX_RTFLAYOUTSTYLE_Pagination 0x01
#define FX_RTFLAYOUTSTYLE_ExpandTab 0x10

enum class CFX_RTFLineAlignment {
  Left = 0,
  Center,
  Right,
  Justified,
  Distributed
};

struct FX_RTFTEXTOBJ {
  FX_RTFTEXTOBJ();
  ~FX_RTFTEXTOBJ();

  CFX_WideString pStr;
  std::vector<int32_t> pWidths;
  CFX_RetainPtr<CFGAS_GEFont> pFont;
  const CFX_RectF* pRect;
  wchar_t wLineBreakChar;
  float fFontSize;
  int32_t iLength;
  int32_t iBidiLevel;
  int32_t iHorizontalScale;
  int32_t iVerticalScale;
};

class CFX_RTFBreak {
 public:
  explicit CFX_RTFBreak(uint32_t dwLayoutStyles);
  ~CFX_RTFBreak();

  void SetLineBoundary(float fLineStart, float fLineEnd);
  void SetLineStartPos(float fLinePos);
  void SetFont(const CFX_RetainPtr<CFGAS_GEFont>& pFont);
  void SetFontSize(float fFontSize);
  void SetTabWidth(float fTabWidth);
  void SetLineBreakTolerance(float fTolerance);
  void SetHorizontalScale(int32_t iScale);
  void SetVerticalScale(int32_t iScale);
  void SetCharSpace(float fCharSpace);
  void SetAlignment(CFX_RTFLineAlignment align) { m_iAlignment = align; }
  void SetUserData(const CFX_RetainPtr<CFX_Retainable>& pUserData);

  void AddPositionedTab(float fTabPos);

  CFX_BreakType EndBreak(CFX_BreakType dwStatus);
  int32_t CountBreakPieces() const;
  const CFX_BreakPiece* GetBreakPieceUnstable(int32_t index) const;
  void ClearBreakPieces();

  void Reset();

  int32_t GetDisplayPos(const FX_RTFTEXTOBJ* pText,
                        FXTEXT_CHARPOS* pCharPos,
                        bool bCharCode) const;

  CFX_BreakType AppendChar(wchar_t wch);

  CFX_BreakLine* GetCurrentLineForTesting() const { return m_pCurLine; }

 private:
  void AppendChar_Combination(CFX_Char* pCurChar);
  void AppendChar_Tab(CFX_Char* pCurChar);
  CFX_BreakType AppendChar_Control(CFX_Char* pCurChar);
  CFX_BreakType AppendChar_Arabic(CFX_Char* pCurChar);
  CFX_BreakType AppendChar_Others(CFX_Char* pCurChar);
  void FontChanged();
  void SetBreakStatus();
  CFX_Char* GetLastChar(int32_t index) const;
  bool HasRTFLine() const { return m_iReadyLineIndex >= 0; }
  FX_CHARTYPE GetUnifiedCharType(FX_CHARTYPE chartype) const;
  int32_t GetLastPositionedTab() const;
  bool GetPositionedTab(int32_t* iTabPos) const;

  int32_t GetBreakPos(std::vector<CFX_Char>& tca,
                      int32_t& iEndPos,
                      bool bAllChars,
                      bool bOnlyBrk);
  void SplitTextLine(CFX_BreakLine* pCurLine,
                     CFX_BreakLine* pNextLine,
                     bool bAllChars);
  bool EndBreak_SplitLine(CFX_BreakLine* pNextLine,
                          bool bAllChars,
                          CFX_BreakType dwStatus);
  void EndBreak_BidiLine(std::deque<FX_TPO>* tpos, CFX_BreakType dwStatus);
  void EndBreak_Alignment(const std::deque<FX_TPO>& tpos,
                          bool bAllChars,
                          CFX_BreakType dwStatus);

  int32_t m_iBoundaryStart;
  int32_t m_iBoundaryEnd;
  uint32_t m_dwLayoutStyles;
  bool m_bPagination;
  CFX_RetainPtr<CFGAS_GEFont> m_pFont;
  int32_t m_iFontHeight;
  int32_t m_iFontSize;
  int32_t m_iTabWidth;
  std::vector<int32_t> m_PositionedTabs;
  wchar_t m_wDefChar;
  int32_t m_iDefChar;
  wchar_t m_wLineBreakChar;
  int32_t m_iHorizontalScale;
  int32_t m_iVerticalScale;
  int32_t m_iCharSpace;
  CFX_RTFLineAlignment m_iAlignment;
  CFX_RetainPtr<CFX_Retainable> m_pUserData;
  FX_CHARTYPE m_eCharType;
  uint32_t m_dwIdentity;
  CFX_BreakLine m_RTFLine[2];
  CFX_BreakLine* m_pCurLine;
  int32_t m_iTolerance;
  int8_t m_iReadyLineIndex;
};

#endif  // XFA_FGAS_LAYOUT_FGAS_RTFBREAK_H_
