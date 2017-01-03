// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_WIDGETTP_H_
#define XFA_FWL_THEME_CFWL_WIDGETTP_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fwl/fwl_error.h"
#include "xfa/fwl/theme/cfwl_utils.h"
#include "xfa/fxgraphics/cfx_graphics.h"

class CFDE_TextOut;
class CFGAS_GEFont;
class CFWL_ThemeBackground;
class CFWL_ThemePart;
class CFWL_ThemeText;
class CFGAS_FontMgr;
class CFWL_Widget;

#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
class CFX_FontSourceEnum_File;
#endif

class CFWL_WidgetTP {
 public:
  virtual ~CFWL_WidgetTP();

  virtual void Initialize();
  virtual void Finalize();

  virtual bool IsValidWidget(CFWL_Widget* pWidget);

  virtual void DrawBackground(CFWL_ThemeBackground* pParams);
  virtual void DrawText(CFWL_ThemeText* pParams);
  virtual void CalcTextRect(CFWL_ThemeText* pParams, CFX_RectF& rect);

  CFGAS_GEFont* GetFont() const { return m_pFDEFont; }

  void SetFont(CFWL_Widget* pWidget,
               const FX_WCHAR* strFont,
               FX_FLOAT fFontSize,
               FX_ARGB rgbFont);
  void SetFont(CFWL_Widget* pWidget,
               CFGAS_GEFont* pFont,
               FX_FLOAT fFontSize,
               FX_ARGB rgbFont);
  CFGAS_GEFont* GetFont(CFWL_Widget* pWidget);

 protected:
  CFWL_WidgetTP();

  void InitTTO();
  void FinalizeTTO();

  void Draw3DRect(CFX_Graphics* pGraphics,
                  FWLTHEME_EDGE eType,
                  FX_FLOAT fWidth,
                  const CFX_RectF* pRect,
                  FX_ARGB cr1,
                  FX_ARGB cr2,
                  FX_ARGB cr3,
                  FX_ARGB cr4,
                  CFX_Matrix* pMatrix = nullptr);
  void Draw3DCircle(CFX_Graphics* pGraphics,
                    FWLTHEME_EDGE eType,
                    FX_FLOAT fWidth,
                    const CFX_RectF* pRect,
                    FX_ARGB cr1,
                    FX_ARGB cr2,
                    FX_ARGB cr3,
                    FX_ARGB cr4,
                    CFX_Matrix* pMatrix = nullptr);
  void DrawBorder(CFX_Graphics* pGraphics,
                  const CFX_RectF* pRect,
                  CFX_Matrix* pMatrix = nullptr);
  void FillBackground(CFX_Graphics* pGraphics,
                      const CFX_RectF* pRect,
                      CFX_Matrix* pMatrix = nullptr);
  void FillSoildRect(CFX_Graphics* pGraphics,
                     FX_ARGB fillColor,
                     const CFX_RectF* pRect,
                     CFX_Matrix* pMatrix = nullptr);
  void DrawAxialShading(CFX_Graphics* pGraphics,
                        FX_FLOAT fx1,
                        FX_FLOAT fy1,
                        FX_FLOAT fx2,
                        FX_FLOAT fy2,
                        FX_ARGB beginColor,
                        FX_ARGB endColor,
                        CFX_Path* path,
                        int32_t fillMode = FXFILL_WINDING,
                        CFX_Matrix* pMatrix = nullptr);
  void DrawAnnulusRect(CFX_Graphics* pGraphics,
                       FX_ARGB fillColor,
                       const CFX_RectF* pRect,
                       FX_FLOAT fRingWidth = 1,
                       CFX_Matrix* pMatrix = nullptr);
  void DrawAnnulusCircle(CFX_Graphics* pGraphics,
                         FX_ARGB fillColor,
                         const CFX_RectF* pRect,
                         FX_FLOAT fWidth = 1,
                         CFX_Matrix* pMatrix = nullptr);
  void DrawFocus(CFX_Graphics* pGraphics,
                 const CFX_RectF* pRect,
                 CFX_Matrix* pMatrix = nullptr);
  void DrawArrow(CFX_Graphics* pGraphics,
                 const CFX_RectF* pRect,
                 FWLTHEME_DIRECTION eDict,
                 FX_ARGB argbFill,
                 bool bPressed,
                 CFX_Matrix* pMatrix = nullptr);
  void DrawArrow(CFX_Graphics* pGraphics,
                 const CFX_RectF* pRect,
                 FWLTHEME_DIRECTION eDict,
                 FX_ARGB argSign,
                 CFX_Matrix* pMatrix = nullptr);
  void DrawBtn(CFX_Graphics* pGraphics,
               const CFX_RectF* pRect,
               FWLTHEME_STATE eState,
               CFX_Matrix* pMatrix = nullptr);
  void DrawArrowBtn(CFX_Graphics* pGraphics,
                    const CFX_RectF* pRect,
                    FWLTHEME_DIRECTION eDict,
                    FWLTHEME_STATE eState,
                    CFX_Matrix* pMatrix = nullptr);
  uint32_t m_dwRefCount;
  std::unique_ptr<CFDE_TextOut> m_pTextOut;
  CFGAS_GEFont* m_pFDEFont;
};

void FWLTHEME_Release();

class CFWL_FontData {
 public:
  CFWL_FontData();
  virtual ~CFWL_FontData();

  bool Equal(const CFX_WideStringC& wsFontFamily,
             uint32_t dwFontStyles,
             uint16_t wCodePage);
  bool LoadFont(const CFX_WideStringC& wsFontFamily,
                uint32_t dwFontStyles,
                uint16_t wCodePage);
  CFGAS_GEFont* GetFont() const { return m_pFont.get(); }

 protected:
  CFX_WideString m_wsFamily;
  uint32_t m_dwStyles;
  uint32_t m_dwCodePage;
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
  std::unique_ptr<CFX_FontSourceEnum_File> m_pFontSource;
#endif
  std::unique_ptr<CFGAS_FontMgr> m_pFontMgr;
  std::unique_ptr<CFGAS_GEFont> m_pFont;
};

class CFWL_FontManager {
 public:
  static CFWL_FontManager* GetInstance();
  static void DestroyInstance();

  CFGAS_GEFont* FindFont(const CFX_WideStringC& wsFontFamily,
                         uint32_t dwFontStyles,
                         uint16_t dwCodePage);

 protected:
  CFWL_FontManager();
  virtual ~CFWL_FontManager();

  static CFWL_FontManager* s_FontManager;
  std::vector<std::unique_ptr<CFWL_FontData>> m_FontsArray;
};

#endif  // XFA_FWL_THEME_CFWL_WIDGETTP_H_
