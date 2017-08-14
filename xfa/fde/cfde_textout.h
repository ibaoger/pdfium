// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_TEXTOUT_H_
#define XFA_FDE_CFDE_TEXTOUT_H_

#include <deque>
#include <memory>
#include <vector>

#include "core/fxcrt/cfx_char.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/fx_dib.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"

class CFDE_RenderDevice;
class CFX_RenderDevice;
class CFX_TxtBreak;
struct FX_TXTRUN;

enum class FDE_TextAlignment : uint8_t {
  kTopLeft = 0,
  kCenterLeft,
  kCenter,
  kCenterRight
};

struct FDE_TextStyle {
  FDE_TextStyle()
      : single_line_(false), line_wrap_(false), last_line_height_(false) {}
  ~FDE_TextStyle() {}

  void Reset() {
    single_line_ = false;
    line_wrap_ = false;
    last_line_height_ = false;
  }

  bool single_line_;
  bool line_wrap_;
  bool last_line_height_;
};

struct FDE_TTOPIECE {
  FDE_TTOPIECE();
  FDE_TTOPIECE(const FDE_TTOPIECE& that);
  ~FDE_TTOPIECE();

  int32_t iStartChar;
  int32_t iChars;
  uint32_t dwCharStyles;
  CFX_RectF rtPiece;
};

class CFDE_TTOLine {
 public:
  CFDE_TTOLine();
  CFDE_TTOLine(const CFDE_TTOLine& ttoLine);
  ~CFDE_TTOLine();

  bool GetNewReload() const { return m_bNewReload; }
  void SetNewReload(bool reload) { m_bNewReload = reload; }
  int32_t AddPiece(int32_t index, const FDE_TTOPIECE& ttoPiece);
  int32_t GetSize() const;
  FDE_TTOPIECE* GetPtrAt(int32_t index);
  void RemoveLast(int32_t iCount);
  void RemoveAll();

 private:
  bool m_bNewReload;
  std::deque<FDE_TTOPIECE> m_pieces;
};

class CFDE_TextOut {
 public:
  CFDE_TextOut();
  ~CFDE_TextOut();

  void SetFont(const CFX_RetainPtr<CFGAS_GEFont>& pFont);
  void SetFontSize(float fFontSize);
  void SetTextColor(FX_ARGB color);
  void SetStyles(const FDE_TextStyle& dwStyles);
  void SetTabWidth(float fTabWidth);
  void SetParagraphBreakChar(wchar_t wch);
  void SetAlignment(FDE_TextAlignment iAlignment);
  void SetLineSpace(float fLineSpace);
  void SetDIBitmap(const CFX_RetainPtr<CFX_DIBitmap>& pDIB);
  void SetRenderDevice(CFX_RenderDevice* pDevice);
  void SetClipRect(const CFX_Rect& rtClip);
  void SetClipRect(const CFX_RectF& rtClip);
  void SetMatrix(const CFX_Matrix& matrix);
  void SetLineBreakTolerance(float fTolerance);

  void DrawText(const wchar_t* pwsStr, int32_t iLength, int32_t x, int32_t y);
  void DrawText(const wchar_t* pwsStr, int32_t iLength, float x, float y);
  void DrawText(const wchar_t* pwsStr, int32_t iLength, const CFX_Rect& rect);
  void DrawText(const wchar_t* pwsStr, int32_t iLength, const CFX_RectF& rect);

  void SetLogicClipRect(const CFX_RectF& rtClip);
  void CalcLogicSize(const wchar_t* pwsStr, int32_t iLength, CFX_SizeF& size);
  void CalcLogicSize(const wchar_t* pwsStr, int32_t iLength, CFX_RectF& rect);
  void DrawLogicText(const wchar_t* pwsStr, int32_t iLength, float x, float y);
  void DrawLogicText(const wchar_t* pwsStr,
                     int32_t iLength,
                     const CFX_RectF& rect);
  int32_t GetTotalLines();

 private:
  void CalcTextSize(const wchar_t* pwsStr, int32_t iLength, CFX_RectF& rect);
  bool RetrieveLineWidth(CFX_BreakType dwBreakStatus,
                         float& fStartPos,
                         float& fWidth,
                         float& fHeight);
  void SetLineWidth(CFX_RectF& rect);
  void DrawText(const wchar_t* pwsStr,
                int32_t iLength,
                const CFX_RectF& rect,
                const CFX_RectF& rtClip);
  void LoadText(const wchar_t* pwsStr, int32_t iLength, const CFX_RectF& rect);

  void Reload(const CFX_RectF& rect);
  void ReloadLinePiece(CFDE_TTOLine* pLine, const CFX_RectF& rect);
  bool RetrievePieces(CFX_BreakType dwBreakStatus,
                      int32_t& iStartChar,
                      int32_t& iPieceWidths,
                      bool bReload,
                      const CFX_RectF& rect);
  void AppendPiece(const FDE_TTOPIECE& ttoPiece, bool bNeedReload, bool bEnd);
  void DoAlignment(const CFX_RectF& rect);
  void OnDraw(const CFX_RectF& rtClip);
  int32_t GetDisplayPos(FDE_TTOPIECE* pPiece);
  int32_t GetCharRects(const FDE_TTOPIECE* pPiece);

  FX_TXTRUN ToTextRun(const FDE_TTOPIECE* pPiece);

  std::unique_ptr<CFX_TxtBreak> m_pTxtBreak;
  CFX_RetainPtr<CFGAS_GEFont> m_pFont;
  float m_fFontSize;
  float m_fLineSpace;
  float m_fLinePos;
  float m_fTolerance;
  FDE_TextAlignment m_iAlignment;
  FDE_TextStyle m_Styles;
  int32_t m_iTxtBkAlignment;
  std::vector<int32_t> m_CharWidths;
  wchar_t m_wParagraphBkChar;
  FX_ARGB m_TxtColor;
  uint32_t m_dwTxtBkStyles;
  CFX_WideString m_wsText;
  CFX_RectF m_rtClip;
  CFX_RectF m_rtLogicClip;
  CFX_Matrix m_Matrix;
  std::deque<CFDE_TTOLine> m_ttoLines;
  int32_t m_iCurLine;
  int32_t m_iCurPiece;
  int32_t m_iTotalLines;
  std::vector<FXTEXT_CHARPOS> m_CharPos;
  // NOTE: m_pDefaultRenderDevice must outlive m_pRenderDevice.
  std::unique_ptr<CFX_DefaultRenderDevice> m_pDefaultRenderDevice;
  std::unique_ptr<CFDE_RenderDevice> m_pRenderDevice;
  std::vector<CFX_RectF> m_rectArray;
};

#endif  // XFA_FDE_CFDE_TEXTOUT_H_
