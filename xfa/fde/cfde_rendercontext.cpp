// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/cfde_rendercontext.h"

#include <stack>

#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fde/cfde_renderdevice.h"
#include "xfa/fde/cfde_txtedttextset.h"

struct FDE_CANVASITEM {
  CFDE_TxtEdtPage* pCanvas;
  size_t pos;
};

CFDE_RenderContext::CFDE_RenderContext(CFDE_RenderDevice* pRenderDevice,
                                       const CFX_Matrix& tmDoc2Device)
    : m_pRenderDevice(pRenderDevice), m_Transform(tmDoc2Device) {}

CFDE_RenderContext::~CFDE_RenderContext() {}

void CFDE_RenderContext::Render(CFDE_TxtEdtPage* pCanvasSet) {
  if (!m_pRenderDevice || !pCanvasSet || pCanvasSet->GetFirstPosition() == 0)
    return;

  auto canvas_stack = std::stack<FDE_CANVASITEM>();

  canvas_stack.push(
      {.pCanvas = pCanvasSet, .pos = pCanvasSet->GetFirstPosition()});

  CFX_RectF rtDocClip = m_pRenderDevice->GetClipRect();
  if (rtDocClip.IsEmpty()) {
    rtDocClip.left = rtDocClip.top = 0;
    rtDocClip.width = (float)m_pRenderDevice->GetWidth();
    rtDocClip.height = (float)m_pRenderDevice->GetHeight();
  }
  m_Transform.GetInverse().TransformRect(rtDocClip);

  while (!canvas_stack.empty()) {
    FDE_CANVASITEM* item = &canvas_stack.top();
    if (item->pos == 0) {
      if (canvas_stack.size() == 1)
        return;
      canvas_stack.pop();
      continue;
    }

    do {
      FDE_TEXTEDITPIECE* pObj = item->pCanvas->GetNext(&item->pos);
      ASSERT(pObj);

      CFDE_TxtEdtTextSet* pVisualSet = item->pCanvas->GetTextSet();
      if (!pVisualSet)
        return;
      if (rtDocClip.IntersectWith(pVisualSet->GetRect(*pObj))) {
        RenderText(pVisualSet, pObj);
        break;
      }
    } while (item->pos != 0);
  }
}

void CFDE_RenderContext::RenderText(CFDE_TxtEdtTextSet* pTextSet,
                                    FDE_TEXTEDITPIECE* pText) {
  ASSERT(m_pRenderDevice);
  ASSERT(pTextSet && pText);

  CFX_RetainPtr<CFGAS_GEFont> pFont = pTextSet->GetFont();
  if (!pFont)
    return;

  int32_t iCount = pTextSet->GetDisplayPos(*pText, nullptr, false);
  if (iCount < 1)
    return;
  if (m_CharPos.size() < static_cast<size_t>(iCount))
    m_CharPos.resize(iCount, FXTEXT_CHARPOS());

  iCount = pTextSet->GetDisplayPos(*pText, m_CharPos.data(), false);
  m_pRenderDevice->DrawString(pTextSet->GetFontColor(), pFont, m_CharPos.data(),
                              iCount, pTextSet->GetFontSize(), &m_Transform);
}
