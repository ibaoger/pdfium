// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/cfde_visualsetiterator.h"

#include "xfa/fde/cfde_txtedtpage.h"

CFDE_VisualSetIterator::CFDE_VisualSetIterator(CFDE_TxtEdtPage* pCanvas)
    : is_initialized_(false) {
  if (!pCanvas)
    return;

  m_CanvasStack = std::stack<FDE_CANVASITEM>();

  FDE_CANVASITEM canvas;
  canvas.hCanvas = nullptr;
  canvas.pCanvas = pCanvas;
  canvas.pos = pCanvas->GetFirstPosition();
  if (canvas.pos == 0)
    return;

  m_CanvasStack.push(canvas);
  is_initialized_ = true;
}

CFDE_VisualSetIterator::~CFDE_VisualSetIterator() {}

FDE_TEXTEDITPIECE* CFDE_VisualSetIterator::GetNext(
    IFDE_VisualSet*& pVisualSet,
    FDE_TEXTEDITPIECE** phCanvasObj,
    CFDE_TxtEdtPage** ppCanvasSet) {
  if (!is_initialized_) {
    if (ppCanvasSet)
      *ppCanvasSet = nullptr;
    if (phCanvasObj)
      *phCanvasObj = nullptr;
    return nullptr;
  }

  while (!m_CanvasStack.empty()) {
    FDE_CANVASITEM* pCanvas = &m_CanvasStack.top();
    if (pCanvas->pos == 0) {
      if (m_CanvasStack.size() == 1)
        break;

      m_CanvasStack.pop();
      continue;
    }
    do {
      FDE_TEXTEDITPIECE* pObj =
          pCanvas->pCanvas->GetNext(&pCanvas->pos, pVisualSet);
      ASSERT(pObj);

      if (ppCanvasSet)
        *ppCanvasSet = pCanvas->pCanvas;
      if (phCanvasObj)
        *phCanvasObj = pCanvas->hCanvas;
      return pObj;
    } while (pCanvas->pos != 0);
  }
  if (ppCanvasSet)
    *ppCanvasSet = nullptr;
  if (phCanvasObj)
    *phCanvasObj = nullptr;

  pVisualSet = nullptr;
  return nullptr;
}
