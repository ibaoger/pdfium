// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_RENDERCONTEXT_H_
#define XFA_FDE_CFDE_RENDERCONTEXT_H_

#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "xfa/fde/cfde_txtedtpage.h"

class CFDE_RenderDevice;
class CFDE_TxtEdtTextSet;

class CFDE_RenderContext {
 public:
  CFDE_RenderContext(CFDE_RenderDevice* pRenderDevice,
                     const CFX_Matrix& tmDoc2Device);
  ~CFDE_RenderContext();

  void Render(CFDE_TxtEdtPage* pCanvasSet);

 private:
  void RenderText(CFDE_TxtEdtTextSet* pTextSet, FDE_TEXTEDITPIECE* pText);

  CFDE_RenderDevice* m_pRenderDevice;
  CFX_Matrix m_Transform;
  std::vector<FXTEXT_CHARPOS> m_CharPos;
};

#endif  // XFA_FDE_CFDE_RENDERCONTEXT_H_
