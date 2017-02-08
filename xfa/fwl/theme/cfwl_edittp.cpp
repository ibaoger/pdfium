// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_edittp.h"

#include "xfa/fwl/cfwl_edit.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fxfa/app/xfa_fwltheme.h"
#include "xfa/fxfa/xfa_ffwidget.h"
#include "xfa/fxgraphics/cfx_color.h"
#include "xfa/fxgraphics/cfx_path.h"

CFWL_EditTP::CFWL_EditTP() {}

CFWL_EditTP::~CFWL_EditTP() {}

void CFWL_EditTP::DrawBackground(CFWL_ThemeBackground* pParams) {
  if (CFWL_Part::CombTextLine == pParams->m_iPart) {
    CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pParams->m_pWidget);
    FX_ARGB cr = 0xFF000000;
    FX_FLOAT fWidth = 1.0f;
    if (CXFA_Border borderUI = pWidget->GetDataAcc()->GetUIBorder()) {
      CXFA_Edge edge = borderUI.GetEdge(0);
      if (edge) {
        cr = edge.GetColor();
        fWidth = edge.GetThickness();
      }
    }
    CFX_Color crLine(cr);
    pParams->m_pGraphics->SetStrokeColor(&crLine);
    pParams->m_pGraphics->SetLineWidth(fWidth);
    pParams->m_pGraphics->StrokePath(pParams->m_pPath, pParams->m_matrix);
    return;
  }

  switch (pParams->m_iPart) {
    case CFWL_Part::Border: {
      DrawBorder(pParams->m_pGraphics, &pParams->m_rtPart, pParams->m_matrix);
      break;
    }
    case CFWL_Part::Background: {
      if (pParams->m_pPath) {
        CFX_Graphics* pGraphics = pParams->m_pGraphics;
        pGraphics->SaveGraphState();
        CFX_Color crSelected(FWLTHEME_COLOR_BKSelected);
        pGraphics->SetFillColor(&crSelected);
        pGraphics->FillPath(pParams->m_pPath, FXFILL_WINDING,
                            pParams->m_matrix);
        pGraphics->RestoreGraphState();
      } else {
        CFX_Path path;
        path.Create();
        path.AddRectangle(pParams->m_rtPart.left, pParams->m_rtPart.top,
                          pParams->m_rtPart.width, pParams->m_rtPart.height);
        CFX_Color cr(FWLTHEME_COLOR_Background);
        if (!pParams->m_bStaticBackground) {
          if (pParams->m_dwStates & CFWL_PartState_Disabled)
            cr.Set(FWLTHEME_COLOR_EDGERB1);
          else if (pParams->m_dwStates & CFWL_PartState_ReadOnly)
            cr.Set(ArgbEncode(255, 236, 233, 216));
          else
            cr.Set(0xFFFFFFFF);
        }
        pParams->m_pGraphics->SaveGraphState();
        pParams->m_pGraphics->SetFillColor(&cr);
        pParams->m_pGraphics->FillPath(&path, FXFILL_WINDING,
                                       pParams->m_matrix);
        pParams->m_pGraphics->RestoreGraphState();
      }
      break;
    }
    case CFWL_Part::CombTextLine: {
      FX_ARGB cr = 0xFF000000;
      FX_FLOAT fWidth = 1.0f;
      CFX_Color crLine(cr);
      pParams->m_pGraphics->SetStrokeColor(&crLine);
      pParams->m_pGraphics->SetLineWidth(fWidth);
      pParams->m_pGraphics->StrokePath(pParams->m_pPath, pParams->m_matrix);
      break;
    }
    default:
      break;
  }
}
