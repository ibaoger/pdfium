// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_VISUALSETITERATOR_H_
#define XFA_FDE_CFDE_VISUALSETITERATOR_H_

#include <stack>

#include "xfa/fde/ifde_visualset.h"

class CFDE_TxtEdtPage;

struct FDE_CANVASITEM {
  CFDE_TxtEdtPage* pCanvas;
  FDE_TEXTEDITPIECE* hCanvas;
  size_t pos;
};

class CFDE_VisualSetIterator {
 public:
  explicit CFDE_VisualSetIterator(CFDE_TxtEdtPage* pCanvas);
  ~CFDE_VisualSetIterator();

  // void Reset();
  FDE_TEXTEDITPIECE* GetNext(IFDE_VisualSet*& pVisualSet,
                             FDE_TEXTEDITPIECE** phCanvasObj = nullptr,
                             CFDE_TxtEdtPage** ppCanvasSet = nullptr);

 protected:
  bool is_initialized_;
  std::stack<FDE_CANVASITEM> m_CanvasStack;
};

#endif  // XFA_FDE_CFDE_VISUALSETITERATOR_H_
