// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_CPDFSDK_APPSTREAM_H_
#define FPDFSDK_CPDFSDK_APPSTREAM_H_

#include "core/fpdfdoc/cpvt_wordrange.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"

class CFX_Edit;
struct CPWL_Color;

// checkbox & radiobutton style
#define PCS_CHECK 0
#define PCS_CIRCLE 1
#define PCS_CROSS 2
#define PCS_DIAMOND 3
#define PCS_SQUARE 4
#define PCS_STAR 5

// pushbutton layout style
#define PPBL_LABEL 0
#define PPBL_ICON 1
#define PPBL_ICONTOPLABELBOTTOM 2
#define PPBL_LABELTOPICONBOTTOM 3
#define PPBL_ICONLEFTLABELRIGHT 4
#define PPBL_LABELLEFTICONRIGHT 5
#define PPBL_LABELOVERICON 6

class CPDFSDK_AppStream {
 public:
  static CFX_ByteString GetColor(const CPWL_Color& color,
                                 const bool& bFillOrStroke = true);
  static CFX_ByteString GetBorder(const CFX_FloatRect& rect,
                                  float fWidth,
                                  const CPWL_Color& color,
                                  const CPWL_Color& crLeftTop,
                                  const CPWL_Color& crRightBottom,
                                  BorderStyle nStyle,
                                  const CPWL_Dash& dash);
  static CFX_ByteString GetCircleBorder(const CFX_FloatRect& rect,
                                        float fWidth,
                                        const CPWL_Color& color,
                                        const CPWL_Color& crLeftTop,
                                        const CPWL_Color& crRightBottom,
                                        BorderStyle nStyle,
                                        const CPWL_Dash& dash);
  static CFX_ByteString GetRectFill(const CFX_FloatRect& rect,
                                    const CPWL_Color& color);
  static CFX_ByteString GetCircleFill(const CFX_FloatRect& rect,
                                      const CPWL_Color& color);
  static CFX_ByteString GetPushButton(const CFX_FloatRect& rcBBox,
                                      IPVT_FontMap* pFontMap,
                                      CPDF_Stream* pIconStream,
                                      CPDF_IconFit& IconFit,
                                      const CFX_WideString& sLabel,
                                      const CPWL_Color& crText,
                                      float fFontSize,
                                      int32_t nLayOut);
  static CFX_ByteString GetCheckBox(const CFX_FloatRect& rcBBox,
                                    int32_t nStyle,
                                    const CPWL_Color& crText);
  static CFX_ByteString GetRadioButton(const CFX_FloatRect& rcBBox,
                                       int32_t nStyle,
                                       const CPWL_Color& crText);
  static CFX_ByteString GetEdit(CFX_Edit* pEdit,
                                const CFX_PointF& ptOffset,
                                const CPVT_WordRange* pRange = nullptr,
                                bool bContinuous = true,
                                uint16_t SubWord = 0);
  static CFX_ByteString GetEditSel(CFX_Edit* pEdit,
                                   const CFX_PointF& ptOffset,
                                   const CPVT_WordRange* pRange = nullptr);
  static CFX_ByteString GetDropButton(const CFX_FloatRect& rcBBox);

 private:
  static CFX_ByteString Get_Check(const CFX_FloatRect& rcBBox,
                                  const CPWL_Color& crText);
  static CFX_ByteString Get_Circle(const CFX_FloatRect& rcBBox,
                                   const CPWL_Color& crText);
  static CFX_ByteString Get_Cross(const CFX_FloatRect& rcBBox,
                                  const CPWL_Color& crText);
  static CFX_ByteString Get_Diamond(const CFX_FloatRect& rcBBox,
                                    const CPWL_Color& crText);
  static CFX_ByteString Get_Square(const CFX_FloatRect& rcBBox,
                                   const CPWL_Color& crText);
  static CFX_ByteString Get_Star(const CFX_FloatRect& rcBBox,
                                 const CPWL_Color& crText);

  static CFX_ByteString GetAP_Check(const CFX_FloatRect& crBBox);
  static CFX_ByteString GetAP_Circle(const CFX_FloatRect& crBBox);
  static CFX_ByteString GetAP_Cross(const CFX_FloatRect& crBBox);
  static CFX_ByteString GetAP_Diamond(const CFX_FloatRect& crBBox);
  static CFX_ByteString GetAP_Square(const CFX_FloatRect& crBBox);
  static CFX_ByteString GetAP_Star(const CFX_FloatRect& crBBox);
  static CFX_ByteString GetAP_HalfCircle(const CFX_FloatRect& crBBox,
                                         float fRotate);
};

#endif  // FPDFSDK_CPDFSDK_APPSTREAM_H_
