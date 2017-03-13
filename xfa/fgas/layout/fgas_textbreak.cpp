// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/fgas_textbreak.h"

#include <algorithm>

#include "core/fxcrt/fx_arabic.h"
#include "core/fxcrt/fx_memory.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/layout/fgas_linebreak.h"

namespace {

typedef CFX_BreakType (CFX_TxtBreak::*FX_TxtBreak_LPFAppendChar)(
    CFX_Char* pCurChar);
const FX_TxtBreak_LPFAppendChar g_FX_TxtBreak_lpfAppendChar[16] = {
    &CFX_TxtBreak::AppendChar_Others,      &CFX_TxtBreak::AppendChar_Tab,
    &CFX_TxtBreak::AppendChar_Others,      &CFX_TxtBreak::AppendChar_Control,
    &CFX_TxtBreak::AppendChar_Combination, &CFX_TxtBreak::AppendChar_Others,
    &CFX_TxtBreak::AppendChar_Others,      &CFX_TxtBreak::AppendChar_Arabic,
    &CFX_TxtBreak::AppendChar_Arabic,      &CFX_TxtBreak::AppendChar_Arabic,
    &CFX_TxtBreak::AppendChar_Arabic,      &CFX_TxtBreak::AppendChar_Arabic,
    &CFX_TxtBreak::AppendChar_Arabic,      &CFX_TxtBreak::AppendChar_Others,
    &CFX_TxtBreak::AppendChar_Others,      &CFX_TxtBreak::AppendChar_Others,
};

bool IsCtrlCode(wchar_t ch) {
  uint32_t dwRet = (FX_GetUnicodeProperties(ch) & charTYPEBITSMASK);
  return dwRet == charTYPE_Tab || dwRet == charTYPE_Control;
}

const int kMinimumTabWidth = 160000;

}  // namespace

CFX_TxtBreak::CFX_TxtBreak()
    : m_iLineWidth(2000000),
      m_dwLayoutStyles(0),
      m_bSingleLine(false),
      m_bCombText(false),
      m_bEquidistant(true),
      m_iArabicContext(1),
      m_iCurArabicContext(1),
      m_pFont(nullptr),
      m_iFontSize(240),
      m_iTabWidth(720000),
      m_wDefChar(0xFEFF),
      m_wParagBreakChar(L'\n'),
      m_iDefChar(0),
      m_iAlignment(CFX_TxtLineAlignment_Left),
      m_dwContextCharStyles(0),
      m_iCombWidth(360000),
      m_eCharType(charTYPE_Unknown),
      m_pCurLine(nullptr),
      m_iTolerance(0),
      m_iHorScale(100),
      m_iCharSpace(0),
      m_iReadyLineIndex(-1) {
  m_pCurLine = &m_TxtLine[0];
  ResetArabicContext();
}

CFX_TxtBreak::~CFX_TxtBreak() {}

void CFX_TxtBreak::SetLineWidth(FX_FLOAT fLineWidth) {
  m_iLineWidth = FXSYS_round(fLineWidth * 20000.0f);
  ASSERT(m_iLineWidth >= 20000);
}

void CFX_TxtBreak::SetLayoutStyles(uint32_t dwLayoutStyles) {
  m_dwLayoutStyles = dwLayoutStyles;
  m_bSingleLine = (m_dwLayoutStyles & FX_TXTLAYOUTSTYLE_SingleLine) != 0;
  m_bCombText = (m_dwLayoutStyles & FX_TXTLAYOUTSTYLE_CombText) != 0;
  ResetArabicContext();
}

void CFX_TxtBreak::SetFont(const CFX_RetainPtr<CFGAS_GEFont>& pFont) {
  if (!pFont || pFont == m_pFont)
    return;

  SetBreakStatus();
  m_pFont = pFont;
  FontChanged();
}

void CFX_TxtBreak::SetFontSize(FX_FLOAT fFontSize) {
  int32_t iFontSize = FXSYS_round(fFontSize * 20.0f);
  if (m_iFontSize == iFontSize)
    return;

  SetBreakStatus();
  m_iFontSize = iFontSize;
  FontChanged();
}

void CFX_TxtBreak::FontChanged() {
  m_iDefChar = 0;
  if (m_wDefChar == 0xFEFF || !m_pFont)
    return;

  m_pFont->GetCharWidth(m_wDefChar, m_iDefChar, false);
  m_iDefChar *= m_iFontSize;
}

void CFX_TxtBreak::SetTabWidth(FX_FLOAT fTabWidth, bool bEquidistant) {
  m_iTabWidth = std::max(FXSYS_round(fTabWidth * 20000.0f), kMinimumTabWidth);
  m_bEquidistant = bEquidistant;
}

void CFX_TxtBreak::SetDefaultChar(wchar_t wch) {
  m_wDefChar = wch;
  m_iDefChar = 0;
  if (m_wDefChar == 0xFEFF || !m_pFont)
    return;

  m_pFont->GetCharWidth(m_wDefChar, m_iDefChar, false);
  if (m_iDefChar < 0)
    m_iDefChar = 0;
  else
    m_iDefChar *= m_iFontSize;
}

void CFX_TxtBreak::SetParagraphBreakChar(wchar_t wch) {
  if (wch != L'\r' && wch != L'\n')
    return;
  m_wParagBreakChar = wch;
}

void CFX_TxtBreak::SetLineBreakTolerance(FX_FLOAT fTolerance) {
  m_iTolerance = FXSYS_round(fTolerance * 20000.0f);
}

void CFX_TxtBreak::SetAlignment(int32_t iAlignment) {
  ASSERT(iAlignment >= CFX_TxtLineAlignment_Left &&
         iAlignment <= CFX_TxtLineAlignment_Justified);
  m_iAlignment = iAlignment;
  ResetArabicContext();
}

void CFX_TxtBreak::ResetContextCharStyles() {
  m_dwContextCharStyles = m_iAlignment;
  m_dwContextCharStyles |= (m_iArabicContext << 8);
}

void CFX_TxtBreak::SetCombWidth(FX_FLOAT fCombWidth) {
  m_iCombWidth = FXSYS_round(fCombWidth * 20000.0f);
}

void CFX_TxtBreak::SetBreakStatus() {
  int32_t iCount = m_pCurLine->CountChars();
  if (iCount < 1)
    return;

  CFX_Char* pTC = m_pCurLine->GetChar(iCount - 1);
  if (pTC->m_dwStatus == CFX_BreakType::None)
    pTC->m_dwStatus = CFX_BreakType::Piece;
}

void CFX_TxtBreak::SetHorizontalScale(int32_t iScale) {
  if (iScale < 0)
    iScale = 0;
  if (iScale == m_iHorScale)
    return;

  SetBreakStatus();
  m_iHorScale = iScale;
}

void CFX_TxtBreak::SetCharSpace(FX_FLOAT fCharSpace) {
  m_iCharSpace = FXSYS_round(fCharSpace * 20000.0f);
}

CFX_Char* CFX_TxtBreak::GetLastChar(int32_t index, bool bOmitChar) const {
  std::vector<CFX_Char>& ca = m_pCurLine->m_LineChars;
  int32_t iCount = pdfium::CollectionSize<int32_t>(ca);
  if (index < 0 || index >= iCount)
    return nullptr;

  int32_t iStart = iCount - 1;
  while (iStart > -1) {
    CFX_Char* pTC = &ca[iStart--];
    if (bOmitChar && pTC->GetCharType() == charTYPE_Combination)
      continue;
    if (--index < 0)
      return pTC;
  }
  return nullptr;
}

inline charTYPE CFX_TxtBreak::GetUnifiedCharType(charTYPE chartype) const {
  return chartype >= charTYPE_ArabicAlef ? charTYPE_Arabic : chartype;
}

void CFX_TxtBreak::ResetArabicContext() {
  m_iCurAlignment = m_iAlignment;
  ResetContextCharStyles();
}

void CFX_TxtBreak::AppendChar_PageLoad(CFX_Char* pCurChar, uint32_t dwProps) {
  pCurChar->m_dwStatus = CFX_BreakType::None;
  pCurChar->m_dwCharStyles = m_dwContextCharStyles;
}

CFX_BreakType CFX_TxtBreak::AppendChar_Combination(CFX_Char* pCurChar) {
  wchar_t wch = pCurChar->m_wCharCode;
  wchar_t wForm;
  int32_t iCharWidth = 0;
  pCurChar->m_iCharWidth = -1;
  if (m_bCombText) {
    iCharWidth = m_iCombWidth;
  } else {
    wForm = wch;
    CFX_Char* pLastChar = GetLastChar(0, false);
    if (pLastChar &&
        (pLastChar->m_dwCharStyles & FX_TXTCHARSTYLE_ArabicShadda) == 0) {
      bool bShadda = false;
      if (wch == 0x0651) {
        wchar_t wLast = pLastChar->m_wCharCode;
        if (wLast >= 0x064C && wLast <= 0x0650) {
          wForm = FX_GetArabicFromShaddaTable(wLast);
          bShadda = true;
        }
      } else if (wch >= 0x064C && wch <= 0x0650) {
        if (pLastChar->m_wCharCode == 0x0651) {
          wForm = FX_GetArabicFromShaddaTable(wch);
          bShadda = true;
        }
      }
      if (bShadda) {
        pLastChar->m_dwCharStyles |= FX_TXTCHARSTYLE_ArabicShadda;
        pLastChar->m_iCharWidth = 0;
        pCurChar->m_dwCharStyles |= FX_TXTCHARSTYLE_ArabicShadda;
      }
    }
    if (!m_pFont->GetCharWidth(wForm, iCharWidth, false))
      iCharWidth = 0;

    iCharWidth *= m_iFontSize;
    iCharWidth = iCharWidth * m_iHorScale / 100;
  }
  pCurChar->m_iCharWidth = -iCharWidth;
  return CFX_BreakType::None;
}

CFX_BreakType CFX_TxtBreak::AppendChar_Tab(CFX_Char* pCurChar) {
  m_eCharType = charTYPE_Tab;
  return CFX_BreakType::None;
}

CFX_BreakType CFX_TxtBreak::AppendChar_Control(CFX_Char* pCurChar) {
  m_eCharType = charTYPE_Control;
  CFX_BreakType dwRet = CFX_BreakType::None;
  if (!m_bSingleLine) {
    wchar_t wch = pCurChar->m_wCharCode;
    switch (wch) {
      case L'\v':
      case 0x2028:
        dwRet = CFX_BreakType::Line;
        break;
      case L'\f':
        dwRet = CFX_BreakType::Page;
        break;
      case 0x2029:
        dwRet = CFX_BreakType::Paragraph;
        break;
      default:
        if (wch == m_wParagBreakChar)
          dwRet = CFX_BreakType::Paragraph;
        break;
    }
    if (dwRet != CFX_BreakType::None)
      dwRet = EndBreak(dwRet);
  }
  return dwRet;
}

CFX_BreakType CFX_TxtBreak::AppendChar_Arabic(CFX_Char* pCurChar) {
  charTYPE chartype = pCurChar->GetCharType();
  int32_t& iLineWidth = m_pCurLine->m_iWidth;
  wchar_t wForm;
  int32_t iCharWidth = 0;
  CFX_Char* pLastChar = nullptr;
  bool bAlef = false;
  if (!m_bCombText && m_eCharType >= charTYPE_ArabicAlef &&
      m_eCharType <= charTYPE_ArabicDistortion) {
    pLastChar = GetLastChar(1);
    if (pLastChar) {
      iCharWidth = pLastChar->m_iCharWidth;
      if (iCharWidth > 0)
        iLineWidth -= iCharWidth;

      CFX_Char* pPrevChar = GetLastChar(2);
      wForm = pdfium::arabic::GetFormChar(pLastChar, pPrevChar, pCurChar);
      bAlef =
          (wForm == 0xFEFF && pLastChar->GetCharType() == charTYPE_ArabicAlef);
      m_pFont->GetCharWidth(wForm, iCharWidth, false);

      if (wForm == 0xFEFF)
        iCharWidth = m_iDefChar;

      iCharWidth *= m_iFontSize;
      iCharWidth = iCharWidth * m_iHorScale / 100;
      pLastChar->m_iCharWidth = iCharWidth;
      iLineWidth += iCharWidth;
      iCharWidth = 0;
    }
  }

  m_eCharType = chartype;
  wForm = pdfium::arabic::GetFormChar(pCurChar, bAlef ? nullptr : pLastChar,
                                      nullptr);
  if (m_bCombText) {
    iCharWidth = m_iCombWidth;
  } else {
    m_pFont->GetCharWidth(wForm, iCharWidth, false);

    if (wForm == 0xFEFF)
      iCharWidth = m_iDefChar;

    iCharWidth *= m_iFontSize;
    iCharWidth = iCharWidth * m_iHorScale / 100;
  }
  pCurChar->m_iCharWidth = iCharWidth;
  iLineWidth += iCharWidth;
  m_pCurLine->m_iArabicChars++;
  if (!m_bSingleLine && iLineWidth > m_iLineWidth + m_iTolerance)
    return EndBreak(CFX_BreakType::Line);
  return CFX_BreakType::None;
}

CFX_BreakType CFX_TxtBreak::AppendChar_Others(CFX_Char* pCurChar) {
  charTYPE chartype = pCurChar->GetCharType();
  int32_t& iLineWidth = m_pCurLine->m_iWidth;
  int32_t iCharWidth = 0;
  m_eCharType = chartype;
  wchar_t wch = pCurChar->m_wCharCode;
  wchar_t wForm = wch;

  if (m_bCombText) {
    iCharWidth = m_iCombWidth;
  } else {
    if (!m_pFont->GetCharWidth(wForm, iCharWidth, false))
      iCharWidth = m_iDefChar;

    iCharWidth *= m_iFontSize;
    iCharWidth = iCharWidth * m_iHorScale / 100;
  }

  iCharWidth += m_iCharSpace;
  pCurChar->m_iCharWidth = iCharWidth;
  iLineWidth += iCharWidth;
  if (!m_bSingleLine && chartype != charTYPE_Space &&
      iLineWidth > m_iLineWidth + m_iTolerance) {
    return EndBreak(CFX_BreakType::Line);
  }

  return CFX_BreakType::None;
}

CFX_BreakType CFX_TxtBreak::AppendChar(wchar_t wch) {
  uint32_t dwProps = kTextLayoutCodeProperties[static_cast<uint16_t>(wch)];
  charTYPE chartype = GetCharTypeFromProp(dwProps);
  m_pCurLine->m_LineChars.emplace_back();

  CFX_Char* pCurChar = &m_pCurLine->m_LineChars.back();
  pCurChar->m_wCharCode = static_cast<uint16_t>(wch);
  pCurChar->m_dwCharProps = dwProps;
  pCurChar->m_dwCharStyles = 0;
  pCurChar->m_iCharWidth = 0;
  pCurChar->m_iHorizontalScale = m_iHorScale;
  pCurChar->m_iVerticalScale = 100;
  pCurChar->m_dwStatus = CFX_BreakType::None;
  pCurChar->m_iBidiClass = 0;
  pCurChar->m_iBidiLevel = 0;
  pCurChar->m_iBidiPos = 0;
  pCurChar->m_iBidiOrder = 0;

  AppendChar_PageLoad(pCurChar, dwProps);
  CFX_BreakType dwRet1 = CFX_BreakType::None;
  if (chartype != charTYPE_Combination &&
      GetUnifiedCharType(m_eCharType) != GetUnifiedCharType(chartype) &&
      m_eCharType != charTYPE_Unknown &&
      m_pCurLine->m_iWidth > m_iLineWidth + m_iTolerance && !m_bSingleLine &&
      (m_eCharType != charTYPE_Space || chartype != charTYPE_Control)) {
    dwRet1 = EndBreak(CFX_BreakType::Line);
    int32_t iCount = m_pCurLine->CountChars();
    if (iCount > 0)
      pCurChar = &m_pCurLine->m_LineChars[iCount - 1];
  }

  CFX_BreakType dwRet2 =
      (this->*g_FX_TxtBreak_lpfAppendChar[chartype >> charTYPEBITS])(pCurChar);
  return std::max(dwRet1, dwRet2);
}

bool CFX_TxtBreak::EndBreak_SplitLine(CFX_BreakLine* pNextLine,
                                      bool bAllChars) {
  int32_t iCount = m_pCurLine->CountChars();
  bool bDone = false;
  CFX_Char* pTC;
  if (!m_bSingleLine && m_pCurLine->m_iWidth > m_iLineWidth + m_iTolerance) {
    pTC = m_pCurLine->GetChar(iCount - 1);
    switch (pTC->GetCharType()) {
      case charTYPE_Tab:
      case charTYPE_Control:
      case charTYPE_Space:
        break;
      default:
        SplitTextLine(m_pCurLine, pNextLine, bAllChars);
        bDone = true;
        break;
    }
  }

  iCount = m_pCurLine->CountChars();
  CFX_BreakPiece tp;
  if (bAllChars && !bDone) {
    int32_t iEndPos = m_pCurLine->m_iWidth;
    GetBreakPos(m_pCurLine->m_LineChars, iEndPos, bAllChars, true);
  }
  return false;
}

void CFX_TxtBreak::EndBreak_BidiLine(std::deque<FX_TPO>* tpos,
                                     CFX_BreakType dwStatus) {
  CFX_BreakPiece tp;
  FX_TPO tpo;
  CFX_Char* pTC;
  int32_t i;
  int32_t j;
  std::vector<CFX_Char>& chars = m_pCurLine->m_LineChars;
  int32_t iCount = m_pCurLine->CountChars();
  bool bDone = m_pCurLine->m_iArabicChars > 0;
  if (bDone) {
    int32_t iBidiNum = 0;
    for (i = 0; i < iCount; i++) {
      pTC = &chars[i];
      pTC->m_iBidiPos = i;
      if (pTC->GetCharType() != charTYPE_Control)
        iBidiNum = i;
      if (i == 0)
        pTC->m_iBidiLevel = 1;
    }
    FX_BidiLine(chars, iBidiNum + 1, 0);
  }

  if (bDone) {
    tp.m_dwStatus = CFX_BreakType::Piece;
    tp.m_iStartPos = m_pCurLine->m_iStart;
    tp.m_pChars = &m_pCurLine->m_LineChars;
    int32_t iBidiLevel = -1;
    int32_t iCharWidth;
    i = 0;
    j = -1;
    while (i < iCount) {
      pTC = &chars[i];
      if (iBidiLevel < 0) {
        iBidiLevel = pTC->m_iBidiLevel;
        tp.m_iWidth = 0;
        tp.m_iBidiLevel = iBidiLevel;
        tp.m_iBidiPos = pTC->m_iBidiOrder;
        tp.m_dwCharStyles = pTC->m_dwCharStyles;
        tp.m_iHorizontalScale = pTC->m_iHorizontalScale;
        tp.m_iVerticalScale = pTC->m_iVerticalScale;
        tp.m_dwStatus = CFX_BreakType::Piece;
      }
      if (iBidiLevel != pTC->m_iBidiLevel ||
          pTC->m_dwStatus != CFX_BreakType::None) {
        if (iBidiLevel == pTC->m_iBidiLevel) {
          tp.m_dwStatus = pTC->m_dwStatus;
          iCharWidth = pTC->m_iCharWidth;
          if (iCharWidth > 0)
            tp.m_iWidth += iCharWidth;

          i++;
        }
        tp.m_iChars = i - tp.m_iStartChar;
        m_pCurLine->m_LinePieces.push_back(tp);
        tp.m_iStartPos += tp.m_iWidth;
        tp.m_iStartChar = i;
        tpo.index = ++j;
        tpo.pos = tp.m_iBidiPos;
        tpos->push_back(tpo);
        iBidiLevel = -1;
      } else {
        iCharWidth = pTC->m_iCharWidth;
        if (iCharWidth > 0)
          tp.m_iWidth += iCharWidth;

        i++;
      }
    }
    if (i > tp.m_iStartChar) {
      tp.m_dwStatus = dwStatus;
      tp.m_iChars = i - tp.m_iStartChar;
      m_pCurLine->m_LinePieces.push_back(tp);
      tpo.index = ++j;
      tpo.pos = tp.m_iBidiPos;
      tpos->push_back(tpo);
    }
    if (j > -1) {
      if (j > 0) {
        std::sort(tpos->begin(), tpos->end());
        int32_t iStartPos = 0;
        for (i = 0; i <= j; i++) {
          tpo = (*tpos)[i];
          CFX_BreakPiece& ttp = m_pCurLine->m_LinePieces[tpo.index];
          ttp.m_iStartPos = iStartPos;
          iStartPos += ttp.m_iWidth;
        }
      }
      m_pCurLine->m_LinePieces[j].m_dwStatus = dwStatus;
    }
  } else {
    tp.m_dwStatus = dwStatus;
    tp.m_iStartPos = m_pCurLine->m_iStart;
    tp.m_iWidth = m_pCurLine->m_iWidth;
    tp.m_iStartChar = 0;
    tp.m_iChars = iCount;
    tp.m_pChars = &m_pCurLine->m_LineChars;
    pTC = &chars[0];
    tp.m_dwCharStyles = pTC->m_dwCharStyles;
    tp.m_iHorizontalScale = pTC->m_iHorizontalScale;
    tp.m_iVerticalScale = pTC->m_iVerticalScale;
    m_pCurLine->m_LinePieces.push_back(tp);
    tpos->push_back({0, 0});
  }
}

void CFX_TxtBreak::EndBreak_Alignment(const std::deque<FX_TPO>& tpos,
                                      bool bAllChars,
                                      CFX_BreakType dwStatus) {
  int32_t iNetWidth = m_pCurLine->m_iWidth;
  int32_t iGapChars = 0;
  bool bFind = false;
  for (auto it = tpos.rbegin(); it != tpos.rend(); ++it) {
    CFX_BreakPiece& ttp = m_pCurLine->m_LinePieces[it->index];
    if (!bFind)
      iNetWidth = ttp.GetEndPos();

    bool bArabic = FX_IsOdd(ttp.m_iBidiLevel);
    int32_t j = bArabic ? 0 : ttp.m_iChars - 1;
    while (j > -1 && j < ttp.m_iChars) {
      const CFX_Char* pTC = ttp.GetChar(j);
      if (pTC->m_nBreakType == FX_LBT_DIRECT_BRK)
        iGapChars++;
      if (!bFind || !bAllChars) {
        charTYPE chartype = pTC->GetCharType();
        if (chartype == charTYPE_Space || chartype == charTYPE_Control) {
          if (!bFind && bAllChars && pTC->m_iCharWidth > 0)
            iNetWidth -= pTC->m_iCharWidth;
        } else {
          bFind = true;
          if (!bAllChars)
            break;
        }
      }
      j += bArabic ? 1 : -1;
    }
    if (!bAllChars && bFind)
      break;
  }

  int32_t iOffset = m_iLineWidth - iNetWidth;
  if (iGapChars > 0 && m_iCurAlignment & CFX_TxtLineAlignment_Justified &&
      dwStatus != CFX_BreakType::Paragraph) {
    int32_t iStart = -1;
    for (auto& tpo : tpos) {
      CFX_BreakPiece& ttp = m_pCurLine->m_LinePieces[tpo.index];
      if (iStart < -1)
        iStart = ttp.m_iStartPos;
      else
        ttp.m_iStartPos = iStart;

      for (int32_t j = 0; j < ttp.m_iChars; j++) {
        CFX_Char* pTC = ttp.GetChar(j);
        if (pTC->m_nBreakType != FX_LBT_DIRECT_BRK || pTC->m_iCharWidth < 0)
          continue;

        int32_t k = iOffset / iGapChars;
        pTC->m_iCharWidth += k;
        ttp.m_iWidth += k;
        iOffset -= k;
        iGapChars--;
        if (iGapChars < 1)
          break;
      }
      iStart += ttp.m_iWidth;
    }
  } else if (m_iCurAlignment & CFX_TxtLineAlignment_Center ||
             m_iCurAlignment & CFX_TxtLineAlignment_Right) {
    if (m_iCurAlignment & CFX_TxtLineAlignment_Center &&
        !(m_iCurAlignment & CFX_TxtLineAlignment_Right)) {
      iOffset /= 2;
    }
    if (iOffset > 0) {
      for (auto& ttp : m_pCurLine->m_LinePieces)
        ttp.m_iStartPos += iOffset;
    }
  }
}

CFX_BreakType CFX_TxtBreak::EndBreak(CFX_BreakType dwStatus) {
  ASSERT(dwStatus != CFX_BreakType::None);

  if (!m_pCurLine->m_LinePieces.empty()) {
    if (dwStatus != CFX_BreakType::Piece)
      m_pCurLine->m_LinePieces.back().m_dwStatus = dwStatus;
    return m_pCurLine->m_LinePieces.back().m_dwStatus;
  }

  if (HasTxtLine()) {
    if (!m_TxtLine[m_iReadyLineIndex].m_LinePieces.empty()) {
      if (dwStatus != CFX_BreakType::Piece)
        m_TxtLine[m_iReadyLineIndex].m_LinePieces.back().m_dwStatus = dwStatus;
      return m_TxtLine[m_iReadyLineIndex].m_LinePieces.back().m_dwStatus;
    }
    return CFX_BreakType::None;
  }

  int32_t iCount = m_pCurLine->CountChars();
  if (iCount < 1)
    return CFX_BreakType::None;

  m_pCurLine->GetChar(iCount - 1)->m_dwStatus = dwStatus;
  if (dwStatus == CFX_BreakType::Piece)
    return dwStatus;

  m_iReadyLineIndex = m_pCurLine == &m_TxtLine[0] ? 0 : 1;
  CFX_BreakLine* pNextLine = &m_TxtLine[1 - m_iReadyLineIndex];
  bool bAllChars = m_iCurAlignment > CFX_TxtLineAlignment_Right;
  if (!EndBreak_SplitLine(pNextLine, bAllChars)) {
    std::deque<FX_TPO> tpos;
    EndBreak_BidiLine(&tpos, dwStatus);
    if (m_iCurAlignment > CFX_TxtLineAlignment_Left)
      EndBreak_Alignment(tpos, bAllChars, dwStatus);
  }

  m_pCurLine = pNextLine;
  CFX_Char* pTC = GetLastChar(0, false);
  m_eCharType = pTC ? pTC->GetCharType() : charTYPE_Unknown;
  if (dwStatus == CFX_BreakType::Paragraph) {
    m_iArabicContext = m_iCurArabicContext = 1;
    ResetArabicContext();
  }
  return dwStatus;
}

int32_t CFX_TxtBreak::GetBreakPos(std::vector<CFX_Char>& ca,
                                  int32_t& iEndPos,
                                  bool bAllChars,
                                  bool bOnlyBrk) {
  int32_t iLength = pdfium::CollectionSize<int32_t>(ca) - 1;
  if (iLength < 1)
    return iLength;

  int32_t iBreak = -1;
  int32_t iBreakPos = -1;
  int32_t iIndirect = -1;
  int32_t iIndirectPos = -1;
  int32_t iLast = -1;
  int32_t iLastPos = -1;
  if (m_bSingleLine || iEndPos <= m_iLineWidth) {
    if (!bAllChars)
      return iLength;

    iBreak = iLength;
    iBreakPos = iEndPos;
  }

  FX_LINEBREAKTYPE eType;
  uint32_t nCodeProp;
  uint32_t nCur;
  uint32_t nNext;
  CFX_Char* pCur = &ca[iLength--];
  if (bAllChars)
    pCur->m_nBreakType = FX_LBT_UNKNOWN;

  nCodeProp = pCur->m_dwCharProps;
  nNext = nCodeProp & 0x003F;
  int32_t iCharWidth = pCur->m_iCharWidth;
  if (iCharWidth > 0)
    iEndPos -= iCharWidth;

  while (iLength >= 0) {
    pCur = &ca[iLength];
    nCodeProp = pCur->m_dwCharProps;
    nCur = nCodeProp & 0x003F;
    if (nCur == FX_CBP_SP) {
      if (nNext == FX_CBP_SP)
        eType = FX_LBT_PROHIBITED_BRK;
      else
        eType = gs_FX_LineBreak_PairTable[nCur][nNext];
    } else {
      if (nNext == FX_CBP_SP)
        eType = FX_LBT_PROHIBITED_BRK;
      else
        eType = gs_FX_LineBreak_PairTable[nCur][nNext];
    }
    if (bAllChars)
      pCur->m_nBreakType = static_cast<uint8_t>(eType);
    if (!bOnlyBrk) {
      if (m_bSingleLine || iEndPos <= m_iLineWidth || nCur == FX_CBP_SP) {
        if (eType == FX_LBT_DIRECT_BRK && iBreak < 0) {
          iBreak = iLength;
          iBreakPos = iEndPos;
          if (!bAllChars)
            return iLength;
        } else if (eType == FX_LBT_INDIRECT_BRK && iIndirect < 0) {
          iIndirect = iLength;
          iIndirectPos = iEndPos;
        }
        if (iLast < 0) {
          iLast = iLength;
          iLastPos = iEndPos;
        }
      }
      iCharWidth = pCur->m_iCharWidth;
      if (iCharWidth > 0)
        iEndPos -= iCharWidth;
    }
    nNext = nCodeProp & 0x003F;
    iLength--;
  }
  if (bOnlyBrk)
    return 0;
  if (iBreak > -1) {
    iEndPos = iBreakPos;
    return iBreak;
  }
  if (iIndirect > -1) {
    iEndPos = iIndirectPos;
    return iIndirect;
  }
  if (iLast > -1) {
    iEndPos = iLastPos;
    return iLast;
  }
  return 0;
}

void CFX_TxtBreak::SplitTextLine(CFX_BreakLine* pCurLine,
                                 CFX_BreakLine* pNextLine,
                                 bool bAllChars) {
  ASSERT(pCurLine && pNextLine);
  int32_t iCount = pCurLine->CountChars();
  if (iCount < 2)
    return;

  int32_t iEndPos = pCurLine->m_iWidth;
  std::vector<CFX_Char>& curChars = pCurLine->m_LineChars;
  int32_t iCharPos = GetBreakPos(curChars, iEndPos, bAllChars, false);
  if (iCharPos < 0)
    iCharPos = 0;

  iCharPos++;
  if (iCharPos >= iCount) {
    pNextLine->Clear();
    CFX_Char* pTC = &curChars[iCharPos - 1];
    pTC->m_nBreakType = FX_LBT_UNKNOWN;
    return;
  }

  pNextLine->m_LineChars =
      std::vector<CFX_Char>(curChars.begin() + iCharPos, curChars.end());
  curChars.erase(curChars.begin() + iCharPos, curChars.end());
  pCurLine->m_iWidth = iEndPos;
  CFX_Char* pTC = &curChars[iCharPos - 1];
  pTC->m_nBreakType = FX_LBT_UNKNOWN;
  iCount = pdfium::CollectionSize<int>(pNextLine->m_LineChars);
  int32_t iWidth = 0;
  for (int32_t i = 0; i < iCount; i++) {
    if (pNextLine->m_LineChars[i].GetCharType() >= charTYPE_ArabicAlef) {
      pCurLine->m_iArabicChars--;
      pNextLine->m_iArabicChars++;
    }
    iWidth += std::max(0, pNextLine->m_LineChars[i].m_iCharWidth);
    pNextLine->m_LineChars[i].m_dwStatus = CFX_BreakType::None;
  }
  pNextLine->m_iWidth = iWidth;
}

int32_t CFX_TxtBreak::CountBreakPieces() const {
  return HasTxtLine() ? pdfium::CollectionSize<int32_t>(
                            m_TxtLine[m_iReadyLineIndex].m_LinePieces)
                      : 0;
}

const CFX_BreakPiece* CFX_TxtBreak::GetBreakPiece(int32_t index) const {
  if (!HasTxtLine())
    return nullptr;
  if (index < 0 ||
      index >= pdfium::CollectionSize<int32_t>(
                   m_TxtLine[m_iReadyLineIndex].m_LinePieces)) {
    return nullptr;
  }
  return &m_TxtLine[m_iReadyLineIndex].m_LinePieces[index];
}

void CFX_TxtBreak::ClearBreakPieces() {
  if (HasTxtLine())
    m_TxtLine[m_iReadyLineIndex].Clear();
  m_iReadyLineIndex = -1;
}

void CFX_TxtBreak::Reset() {
  m_eCharType = charTYPE_Unknown;
  m_iArabicContext = 1;
  m_iCurArabicContext = 1;
  ResetArabicContext();
  m_TxtLine[0].Clear();
  m_TxtLine[1].Clear();
}

struct FX_FORMCHAR {
  uint16_t wch;
  uint16_t wForm;
  int32_t iWidth;
};

int32_t CFX_TxtBreak::GetDisplayPos(const FX_TXTRUN* pTxtRun,
                                    FXTEXT_CHARPOS* pCharPos,
                                    bool bCharCode,
                                    CFX_WideString* pWSForms) const {
  if (!pTxtRun || pTxtRun->iLength < 1)
    return 0;

  CFDE_TxtEdtPage* pAccess = pTxtRun->pAccess;
  const FDE_TEXTEDITPIECE* pIdentity = pTxtRun->pIdentity;
  const wchar_t* pStr = pTxtRun->wsStr.c_str();
  int32_t* pWidths = pTxtRun->pWidths;
  int32_t iLength = pTxtRun->iLength - 1;
  CFX_RetainPtr<CFGAS_GEFont> pFont = pTxtRun->pFont;
  uint32_t dwStyles = pTxtRun->dwStyles;
  CFX_RectF rtText(*pTxtRun->pRect);
  bool bRTLPiece = (pTxtRun->dwCharStyles & FX_TXTCHARSTYLE_OddBidiLevel) != 0;
  FX_FLOAT fFontSize = pTxtRun->fFontSize;
  int32_t iFontSize = FXSYS_round(fFontSize * 20.0f);
  int32_t iAscent = pFont->GetAscent();
  int32_t iDescent = pFont->GetDescent();
  int32_t iMaxHeight = iAscent - iDescent;
  FX_FLOAT fFontHeight = fFontSize;
  FX_FLOAT fAscent = fFontHeight * (FX_FLOAT)iAscent / (FX_FLOAT)iMaxHeight;
  FX_FLOAT fX = rtText.left;
  FX_FLOAT fY;
  FX_FLOAT fCharWidth;
  FX_FLOAT fCharHeight;
  int32_t iHorScale = pTxtRun->iHorizontalScale;
  int32_t iVerScale = pTxtRun->iVerticalScale;
  bool bSkipSpace = pTxtRun->bSkipSpace;
  FX_FORMCHAR formChars[3];
  FX_FLOAT fYBase;

  if (bRTLPiece)
    fX = rtText.right();

  fYBase = rtText.top + (rtText.height - fFontSize) / 2.0f;
  fY = fYBase + fAscent;

  int32_t iCount = 0;
  int32_t iNext = 0;
  wchar_t wPrev = 0xFEFF;
  wchar_t wNext = 0xFEFF;
  wchar_t wForm = 0xFEFF;
  wchar_t wLast = 0xFEFF;
  bool bShadda = false;
  bool bLam = false;
  for (int32_t i = 0; i <= iLength; i++) {
    int32_t iWidth;
    wchar_t wch;
    if (pAccess) {
      wch = pAccess->GetChar(pIdentity, i);
      iWidth = pAccess->GetWidth(pIdentity, i);
    } else {
      wch = *pStr++;
      iWidth = *pWidths++;
    }

    uint32_t dwProps = FX_GetUnicodeProperties(wch);
    charTYPE chartype = GetCharTypeFromProp(dwProps);
    if (chartype == charTYPE_ArabicAlef && iWidth == 0) {
      wPrev = 0xFEFF;
      wLast = wch;
      continue;
    }

    if (chartype >= charTYPE_ArabicAlef) {
      if (i < iLength) {
        if (pAccess) {
          iNext = i + 1;
          while (iNext <= iLength) {
            wNext = pAccess->GetChar(pIdentity, iNext);
            dwProps = FX_GetUnicodeProperties(wNext);
            if ((dwProps & charTYPEBITSMASK) != charTYPE_Combination)
              break;

            iNext++;
          }
          if (iNext > iLength)
            wNext = 0xFEFF;
        } else {
          int32_t j = -1;
          do {
            j++;
            if (i + j >= iLength)
              break;

            wNext = pStr[j];
            dwProps = FX_GetUnicodeProperties(wNext);
          } while ((dwProps & charTYPEBITSMASK) == charTYPE_Combination);
          if (i + j >= iLength)
            wNext = 0xFEFF;
        }
      } else {
        wNext = 0xFEFF;
      }

      wForm = pdfium::arabic::GetFormChar(wch, wPrev, wNext);
      bLam = (wPrev == 0x0644 && wch == 0x0644 && wNext == 0x0647);
    } else if (chartype == charTYPE_Combination) {
      wForm = wch;
      if (wch >= 0x064C && wch <= 0x0651) {
        if (bShadda) {
          wForm = 0xFEFF;
          bShadda = false;
        } else {
          wNext = 0xFEFF;
          if (pAccess) {
            iNext = i + 1;
            if (iNext <= iLength)
              wNext = pAccess->GetChar(pIdentity, iNext);
          } else {
            if (i < iLength)
              wNext = *pStr;
          }
          if (wch == 0x0651) {
            if (wNext >= 0x064C && wNext <= 0x0650) {
              wForm = FX_GetArabicFromShaddaTable(wNext);
              bShadda = true;
            }
          } else {
            if (wNext == 0x0651) {
              wForm = FX_GetArabicFromShaddaTable(wch);
              bShadda = true;
            }
          }
        }
      } else {
        bShadda = false;
      }
    } else if (chartype == charTYPE_Numeric) {
      wForm = wch;
    } else if (wch == L'.') {
      wForm = wch;
    } else if (wch == L',') {
      wForm = wch;
    } else if (bRTLPiece) {
      wForm = FX_GetMirrorChar(wch, dwProps, bRTLPiece, false);
    } else {
      wForm = wch;
    }
    if (chartype != charTYPE_Combination)
      bShadda = false;
    if (chartype < charTYPE_ArabicAlef)
      bLam = false;

    dwProps = FX_GetUnicodeProperties(wForm);
    bool bEmptyChar =
        (chartype >= charTYPE_Tab && chartype <= charTYPE_Control);
    if (wForm == 0xFEFF)
      bEmptyChar = true;

    int32_t iForms = bLam ? 3 : 1;
    iCount += (bEmptyChar && bSkipSpace) ? 0 : iForms;
    if (!pCharPos) {
      if (iWidth > 0)
        wPrev = wch;
      wLast = wch;
      continue;
    }

    int32_t iCharWidth = iWidth;
    if (iCharWidth < 0)
      iCharWidth = -iCharWidth;

    iCharWidth /= iFontSize;
    formChars[0].wch = wch;
    formChars[0].wForm = wForm;
    formChars[0].iWidth = iCharWidth;
    if (bLam) {
      formChars[1].wForm = 0x0651;
      iCharWidth = 0;
      pFont->GetCharWidth(0x0651, iCharWidth, false);
      formChars[1].iWidth = iCharWidth;
      formChars[2].wForm = 0x0670;
      iCharWidth = 0;
      pFont->GetCharWidth(0x0670, iCharWidth, false);
      formChars[2].iWidth = iCharWidth;
    }

    for (int32_t j = 0; j < iForms; j++) {
      wForm = (wchar_t)formChars[j].wForm;
      iCharWidth = formChars[j].iWidth;
      if (j > 0) {
        chartype = charTYPE_Combination;
        wch = wForm;
        wLast = (wchar_t)formChars[j - 1].wForm;
      }
      if (!bEmptyChar || (bEmptyChar && !bSkipSpace)) {
        pCharPos->m_GlyphIndex =
            bCharCode ? wch : pFont->GetGlyphIndex(wForm, false);
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
        pCharPos->m_ExtGID = pCharPos->m_GlyphIndex;
#endif
        pCharPos->m_FontCharWidth = iCharWidth;
        if (pWSForms)
          *pWSForms += wForm;
      }

      int32_t iCharHeight = 1000;

      fCharWidth = fFontSize * iCharWidth / 1000.0f;
      fCharHeight = fFontSize * iCharHeight / 1000.0f;
      if (bRTLPiece && chartype != charTYPE_Combination)
        fX -= fCharWidth;

      if (!bEmptyChar || (bEmptyChar && !bSkipSpace)) {
        pCharPos->m_Origin = CFX_PointF(fX, fY);
        if ((dwStyles & FX_TXTLAYOUTSTYLE_CombText) != 0) {
          int32_t iFormWidth = iCharWidth;
          pFont->GetCharWidth(wForm, iFormWidth, false);
          FX_FLOAT fOffset = fFontSize * (iCharWidth - iFormWidth) / 2000.0f;
          pCharPos->m_Origin.x += fOffset;
        }

        if (chartype == charTYPE_Combination) {
          CFX_Rect rtBBox;
          if (pFont->GetCharBBox(wForm, &rtBBox, false)) {
            pCharPos->m_Origin.y =
                fYBase + fFontSize -
                fFontSize * (FX_FLOAT)rtBBox.height / (FX_FLOAT)iMaxHeight;
          }
          if (wForm == wch && wLast != 0xFEFF) {
            uint32_t dwLastProps = FX_GetUnicodeProperties(wLast);
            if ((dwLastProps & charTYPEBITSMASK) == charTYPE_Combination) {
              CFX_Rect rtBox;
              if (pFont->GetCharBBox(wLast, &rtBox, false))
                pCharPos->m_Origin.y -= fFontSize * rtBox.height / iMaxHeight;
            }
          }
        }
        CFX_PointF ptOffset;
        pCharPos->m_Origin.x += ptOffset.x;
        pCharPos->m_Origin.y -= ptOffset.y;
      }
      if (!bRTLPiece && chartype != charTYPE_Combination)
        fX += fCharWidth;

      if (!bEmptyChar || (bEmptyChar && !bSkipSpace)) {
        pCharPos->m_bGlyphAdjust = true;
        pCharPos->m_AdjustMatrix[0] = -1;
        pCharPos->m_AdjustMatrix[1] = 0;
        pCharPos->m_AdjustMatrix[2] = 0;
        pCharPos->m_AdjustMatrix[3] = 1;

        if (iHorScale != 100 || iVerScale != 100) {
          pCharPos->m_AdjustMatrix[0] =
              pCharPos->m_AdjustMatrix[0] * iHorScale / 100.0f;
          pCharPos->m_AdjustMatrix[1] =
              pCharPos->m_AdjustMatrix[1] * iHorScale / 100.0f;
          pCharPos->m_AdjustMatrix[2] =
              pCharPos->m_AdjustMatrix[2] * iVerScale / 100.0f;
          pCharPos->m_AdjustMatrix[3] =
              pCharPos->m_AdjustMatrix[3] * iVerScale / 100.0f;
        }
        pCharPos++;
      }
    }
    if (iWidth > 0)
      wPrev = static_cast<wchar_t>(formChars[0].wch);
    wLast = wch;
  }
  return iCount;
}

std::vector<CFX_RectF> CFX_TxtBreak::GetCharRects(const FX_TXTRUN* pTxtRun,
                                                  bool bCharBBox) const {
  if (!pTxtRun || pTxtRun->iLength < 1)
    return std::vector<CFX_RectF>();

  CFDE_TxtEdtPage* pAccess = pTxtRun->pAccess;
  const FDE_TEXTEDITPIECE* pIdentity = pTxtRun->pIdentity;
  const wchar_t* pStr = pTxtRun->wsStr.c_str();
  int32_t* pWidths = pTxtRun->pWidths;
  int32_t iLength = pTxtRun->iLength;
  CFX_RectF rect(*pTxtRun->pRect);
  FX_FLOAT fFontSize = pTxtRun->fFontSize;
  int32_t iFontSize = FXSYS_round(fFontSize * 20.0f);
  FX_FLOAT fScale = fFontSize / 1000.0f;
  CFX_RetainPtr<CFGAS_GEFont> pFont = pTxtRun->pFont;
  if (!pFont)
    bCharBBox = false;

  CFX_Rect bbox;
  if (bCharBBox)
    bCharBBox = pFont->GetBBox(&bbox);

  FX_FLOAT fLeft = std::max(0.0f, bbox.left * fScale);
  FX_FLOAT fHeight = FXSYS_fabs(bbox.height * fScale);
  bool bRTLPiece = !!(pTxtRun->dwCharStyles & FX_TXTCHARSTYLE_OddBidiLevel);
  bool bSingleLine = !!(pTxtRun->dwStyles & FX_TXTLAYOUTSTYLE_SingleLine);
  bool bCombText = !!(pTxtRun->dwStyles & FX_TXTLAYOUTSTYLE_CombText);
  wchar_t wch;
  wchar_t wLineBreakChar = pTxtRun->wLineBreakChar;
  int32_t iCharSize;
  FX_FLOAT fCharSize;
  FX_FLOAT fStart = bRTLPiece ? rect.right() : rect.left;

  std::vector<CFX_RectF> rtArray(iLength);
  for (int32_t i = 0; i < iLength; i++) {
    if (pAccess) {
      wch = pAccess->GetChar(pIdentity, i);
      iCharSize = pAccess->GetWidth(pIdentity, i);
    } else {
      wch = *pStr++;
      iCharSize = *pWidths++;
    }
    fCharSize = static_cast<FX_FLOAT>(iCharSize) / 20000.0f;
    bool bRet = (!bSingleLine && IsCtrlCode(wch));
    if (!(wch == L'\v' || wch == L'\f' || wch == 0x2028 || wch == 0x2029 ||
          (wLineBreakChar != 0xFEFF && wch == wLineBreakChar))) {
      bRet = false;
    }
    if (bRet) {
      iCharSize = iFontSize * 500;
      fCharSize = fFontSize / 2.0f;
    }
    rect.left = fStart;
    if (bRTLPiece) {
      rect.left -= fCharSize;
      fStart -= fCharSize;
    } else {
      fStart += fCharSize;
    }
    rect.width = fCharSize;

    if (bCharBBox && !bRet) {
      int32_t iCharWidth = 1000;
      pFont->GetCharWidth(wch, iCharWidth, false);
      FX_FLOAT fRTLeft = 0, fCharWidth = 0;
      if (iCharWidth > 0) {
        fCharWidth = iCharWidth * fScale;
        fRTLeft = fLeft;
        if (bCombText)
          fRTLeft = (rect.width - fCharWidth) / 2.0f;
      }
      CFX_RectF rtBBoxF;
      rtBBoxF.left = rect.left + fRTLeft;
      rtBBoxF.top = rect.top + (rect.height - fHeight) / 2.0f;
      rtBBoxF.width = fCharWidth;
      rtBBoxF.height = fHeight;
      rtBBoxF.top = std::max(rtBBoxF.top, 0.0f);
      rtArray[i] = rtBBoxF;
      continue;
    }
    rtArray[i] = rect;
  }
  return rtArray;
}

FX_TXTRUN::FX_TXTRUN()
    : pAccess(nullptr),
      pIdentity(nullptr),
      pWidths(nullptr),
      iLength(0),
      pFont(nullptr),
      fFontSize(12),
      dwStyles(0),
      iHorizontalScale(100),
      iVerticalScale(100),
      dwCharStyles(0),
      pRect(nullptr),
      wLineBreakChar(L'\n'),
      bSkipSpace(true) {}

FX_TXTRUN::~FX_TXTRUN() {}

FX_TXTRUN::FX_TXTRUN(const FX_TXTRUN& other) = default;
