// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pdfwindow/cpwl_draw_helpers.h"

#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/cfx_renderdevice.h"

void CPWL_DrawHelpers::DrawFillRect(CFX_RenderDevice* pDevice,
                                    CFX_Matrix* pUser2Device,
                                    const CFX_FloatRect& rect,
                                    const FX_COLORREF& color) {
  CFX_PathData path;
  CFX_FloatRect rcTemp(rect);
  path.AppendRect(rcTemp.left, rcTemp.bottom, rcTemp.right, rcTemp.top);
  pDevice->DrawPath(&path, pUser2Device, nullptr, color, 0, FXFILL_WINDING);
}

void CPWL_DrawHelpers::DrawFillArea(CFX_RenderDevice* pDevice,
                                    CFX_Matrix* pUser2Device,
                                    const CFX_PointF* pPts,
                                    int32_t nCount,
                                    const FX_COLORREF& color) {
  CFX_PathData path;
  path.AppendPoint(pPts[0], FXPT_TYPE::MoveTo, false);
  for (int32_t i = 1; i < nCount; i++)
    path.AppendPoint(pPts[i], FXPT_TYPE::LineTo, false);

  pDevice->DrawPath(&path, pUser2Device, nullptr, color, 0, FXFILL_ALTERNATE);
}

void CPWL_DrawHelpers::DrawStrokeRect(CFX_RenderDevice* pDevice,
                                      CFX_Matrix* pUser2Device,
                                      const CFX_FloatRect& rect,
                                      const FX_COLORREF& color,
                                      float fWidth) {
  CFX_PathData path;
  CFX_FloatRect rcTemp(rect);
  path.AppendRect(rcTemp.left, rcTemp.bottom, rcTemp.right, rcTemp.top);

  CFX_GraphStateData gsd;
  gsd.m_LineWidth = fWidth;

  pDevice->DrawPath(&path, pUser2Device, &gsd, 0, color, FXFILL_ALTERNATE);
}

void CPWL_DrawHelpers::DrawStrokeLine(CFX_RenderDevice* pDevice,
                                      CFX_Matrix* pUser2Device,
                                      const CFX_PointF& ptMoveTo,
                                      const CFX_PointF& ptLineTo,
                                      const FX_COLORREF& color,
                                      float fWidth) {
  CFX_PathData path;
  path.AppendPoint(ptMoveTo, FXPT_TYPE::MoveTo, false);
  path.AppendPoint(ptLineTo, FXPT_TYPE::LineTo, false);

  CFX_GraphStateData gsd;
  gsd.m_LineWidth = fWidth;

  pDevice->DrawPath(&path, pUser2Device, &gsd, 0, color, FXFILL_ALTERNATE);
}

void CPWL_DrawHelpers::DrawFillRect(CFX_RenderDevice* pDevice,
                                    CFX_Matrix* pUser2Device,
                                    const CFX_FloatRect& rect,
                                    const CPWL_Color& color,
                                    int32_t nTransparency) {
  CPWL_DrawHelpers::DrawFillRect(pDevice, pUser2Device, rect,
                                 color.ToFXColor(nTransparency));
}

void CPWL_DrawHelpers::DrawShadow(CFX_RenderDevice* pDevice,
                                  CFX_Matrix* pUser2Device,
                                  bool bVertical,
                                  bool bHorizontal,
                                  CFX_FloatRect rect,
                                  int32_t nTransparency,
                                  int32_t nStartGray,
                                  int32_t nEndGray) {
  float fStepGray = 1.0f;

  if (bVertical) {
    fStepGray = (nEndGray - nStartGray) / rect.Height();

    for (float fy = rect.bottom + 0.5f; fy <= rect.top - 0.5f; fy += 1.0f) {
      int32_t nGray = nStartGray + (int32_t)(fStepGray * (fy - rect.bottom));
      CPWL_DrawHelpers::DrawStrokeLine(
          pDevice, pUser2Device, CFX_PointF(rect.left, fy),
          CFX_PointF(rect.right, fy),
          ArgbEncode(nTransparency, nGray, nGray, nGray), 1.5f);
    }
  }

  if (bHorizontal) {
    fStepGray = (nEndGray - nStartGray) / rect.Width();

    for (float fx = rect.left + 0.5f; fx <= rect.right - 0.5f; fx += 1.0f) {
      int32_t nGray = nStartGray + (int32_t)(fStepGray * (fx - rect.left));
      CPWL_DrawHelpers::DrawStrokeLine(
          pDevice, pUser2Device, CFX_PointF(fx, rect.bottom),
          CFX_PointF(fx, rect.top),
          ArgbEncode(nTransparency, nGray, nGray, nGray), 1.5f);
    }
  }
}

void CPWL_DrawHelpers::DrawBorder(CFX_RenderDevice* pDevice,
                                  CFX_Matrix* pUser2Device,
                                  const CFX_FloatRect& rect,
                                  float fWidth,
                                  const CPWL_Color& color,
                                  const CPWL_Color& crLeftTop,
                                  const CPWL_Color& crRightBottom,
                                  BorderStyle nStyle,
                                  int32_t nTransparency) {
  float fLeft = rect.left;
  float fRight = rect.right;
  float fTop = rect.top;
  float fBottom = rect.bottom;

  if (fWidth > 0.0f) {
    float fHalfWidth = fWidth / 2.0f;

    switch (nStyle) {
      default:
      case BorderStyle::SOLID: {
        CFX_PathData path;
        path.AppendRect(fLeft, fBottom, fRight, fTop);
        path.AppendRect(fLeft + fWidth, fBottom + fWidth, fRight - fWidth,
                        fTop - fWidth);
        pDevice->DrawPath(&path, pUser2Device, nullptr,
                          color.ToFXColor(nTransparency), 0, FXFILL_ALTERNATE);
        break;
      }
      case BorderStyle::DASH: {
        CFX_PathData path;
        path.AppendPoint(
            CFX_PointF(fLeft + fWidth / 2.0f, fBottom + fWidth / 2.0f),
            FXPT_TYPE::MoveTo, false);
        path.AppendPoint(
            CFX_PointF(fLeft + fWidth / 2.0f, fTop - fWidth / 2.0f),
            FXPT_TYPE::LineTo, false);
        path.AppendPoint(
            CFX_PointF(fRight - fWidth / 2.0f, fTop - fWidth / 2.0f),
            FXPT_TYPE::LineTo, false);
        path.AppendPoint(
            CFX_PointF(fRight - fWidth / 2.0f, fBottom + fWidth / 2.0f),
            FXPT_TYPE::LineTo, false);
        path.AppendPoint(
            CFX_PointF(fLeft + fWidth / 2.0f, fBottom + fWidth / 2.0f),
            FXPT_TYPE::LineTo, false);

        CFX_GraphStateData gsd;
        gsd.SetDashCount(2);
        gsd.m_DashArray[0] = 3.0f;
        gsd.m_DashArray[1] = 3.0f;
        gsd.m_DashPhase = 0;

        gsd.m_LineWidth = fWidth;
        pDevice->DrawPath(&path, pUser2Device, &gsd, 0,
                          color.ToFXColor(nTransparency), FXFILL_WINDING);
        break;
      }
      case BorderStyle::BEVELED:
      case BorderStyle::INSET: {
        CFX_GraphStateData gsd;
        gsd.m_LineWidth = fHalfWidth;

        CFX_PathData pathLT;

        pathLT.AppendPoint(CFX_PointF(fLeft + fHalfWidth, fBottom + fHalfWidth),
                           FXPT_TYPE::MoveTo, false);
        pathLT.AppendPoint(CFX_PointF(fLeft + fHalfWidth, fTop - fHalfWidth),
                           FXPT_TYPE::LineTo, false);
        pathLT.AppendPoint(CFX_PointF(fRight - fHalfWidth, fTop - fHalfWidth),
                           FXPT_TYPE::LineTo, false);
        pathLT.AppendPoint(
            CFX_PointF(fRight - fHalfWidth * 2, fTop - fHalfWidth * 2),
            FXPT_TYPE::LineTo, false);
        pathLT.AppendPoint(
            CFX_PointF(fLeft + fHalfWidth * 2, fTop - fHalfWidth * 2),
            FXPT_TYPE::LineTo, false);
        pathLT.AppendPoint(
            CFX_PointF(fLeft + fHalfWidth * 2, fBottom + fHalfWidth * 2),
            FXPT_TYPE::LineTo, false);
        pathLT.AppendPoint(CFX_PointF(fLeft + fHalfWidth, fBottom + fHalfWidth),
                           FXPT_TYPE::LineTo, false);

        pDevice->DrawPath(&pathLT, pUser2Device, &gsd,
                          crLeftTop.ToFXColor(nTransparency), 0,
                          FXFILL_ALTERNATE);

        CFX_PathData pathRB;
        pathRB.AppendPoint(CFX_PointF(fRight - fHalfWidth, fTop - fHalfWidth),
                           FXPT_TYPE::MoveTo, false);
        pathRB.AppendPoint(
            CFX_PointF(fRight - fHalfWidth, fBottom + fHalfWidth),
            FXPT_TYPE::LineTo, false);
        pathRB.AppendPoint(CFX_PointF(fLeft + fHalfWidth, fBottom + fHalfWidth),
                           FXPT_TYPE::LineTo, false);
        pathRB.AppendPoint(
            CFX_PointF(fLeft + fHalfWidth * 2, fBottom + fHalfWidth * 2),
            FXPT_TYPE::LineTo, false);
        pathRB.AppendPoint(
            CFX_PointF(fRight - fHalfWidth * 2, fBottom + fHalfWidth * 2),
            FXPT_TYPE::LineTo, false);
        pathRB.AppendPoint(
            CFX_PointF(fRight - fHalfWidth * 2, fTop - fHalfWidth * 2),
            FXPT_TYPE::LineTo, false);
        pathRB.AppendPoint(CFX_PointF(fRight - fHalfWidth, fTop - fHalfWidth),
                           FXPT_TYPE::LineTo, false);

        pDevice->DrawPath(&pathRB, pUser2Device, &gsd,
                          crRightBottom.ToFXColor(nTransparency), 0,
                          FXFILL_ALTERNATE);

        CFX_PathData path;

        path.AppendRect(fLeft, fBottom, fRight, fTop);
        path.AppendRect(fLeft + fHalfWidth, fBottom + fHalfWidth,
                        fRight - fHalfWidth, fTop - fHalfWidth);

        pDevice->DrawPath(&path, pUser2Device, &gsd,
                          color.ToFXColor(nTransparency), 0, FXFILL_ALTERNATE);
        break;
      }
      case BorderStyle::UNDERLINE: {
        CFX_PathData path;
        path.AppendPoint(CFX_PointF(fLeft, fBottom + fWidth / 2),
                         FXPT_TYPE::MoveTo, false);
        path.AppendPoint(CFX_PointF(fRight, fBottom + fWidth / 2),
                         FXPT_TYPE::LineTo, false);

        CFX_GraphStateData gsd;
        gsd.m_LineWidth = fWidth;

        pDevice->DrawPath(&path, pUser2Device, &gsd, 0,
                          color.ToFXColor(nTransparency), FXFILL_ALTERNATE);
        break;
      }
    }
  }
}
