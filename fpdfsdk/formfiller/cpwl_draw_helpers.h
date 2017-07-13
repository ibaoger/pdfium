// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FORMFILLER_CPWL_DRAW_HELPERS_H_
#define FPDFSDK_FORMFILLER_CPWL_DRAW_HELPERS_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxge/fx_dib.h"
#include "fpdfsdk/formfiller/cpwl_color.h"

class CFX_Matrix;
class CFX_RenderDevice;

class CPWL_DrawHelpers {
 public:
  static void DrawFillRect(CFX_RenderDevice* pDevice,
                           CFX_Matrix* pUser2Device,
                           const CFX_FloatRect& rect,
                           const CPWL_Color& color,
                           int32_t nTransparency);
  static void DrawFillRect(CFX_RenderDevice* pDevice,
                           CFX_Matrix* pUser2Device,
                           const CFX_FloatRect& rect,
                           const FX_COLORREF& color);
  static void DrawStrokeRect(CFX_RenderDevice* pDevice,
                             CFX_Matrix* pUser2Device,
                             const CFX_FloatRect& rect,
                             const FX_COLORREF& color,
                             float fWidth);
  static void DrawStrokeLine(CFX_RenderDevice* pDevice,
                             CFX_Matrix* pUser2Device,
                             const CFX_PointF& ptMoveTo,
                             const CFX_PointF& ptLineTo,
                             const FX_COLORREF& color,
                             float fWidth);
  static void DrawBorder(CFX_RenderDevice* pDevice,
                         CFX_Matrix* pUser2Device,
                         const CFX_FloatRect& rect,
                         float fWidth,
                         const CPWL_Color& color,
                         const CPWL_Color& crLeftTop,
                         const CPWL_Color& crRightBottom,
                         BorderStyle nStyle,
                         int32_t nTransparency);
  static void DrawFillArea(CFX_RenderDevice* pDevice,
                           CFX_Matrix* pUser2Device,
                           const CFX_PointF* pPts,
                           int32_t nCount,
                           const FX_COLORREF& color);
  static void DrawShadow(CFX_RenderDevice* pDevice,
                         CFX_Matrix* pUser2Device,
                         bool bVertical,
                         bool bHorizontal,
                         CFX_FloatRect rect,
                         int32_t nTransparency,
                         int32_t nStartGray,
                         int32_t nEndGray);
};

#endif  // FPDFSDK_FORMFILLER_CPWL_DRAW_HELPERS_H_
