// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_CFX_IMAGESTRETCHER_H_
#define CORE_FXGE_DIB_CFX_IMAGESTRETCHER_H_

#include <memory>
#include <vector>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cfx_dibsource.h"
#include "core/fxge/dib/cfx_scanlinecompositor.h"
#include "core/fxge/fx_dib.h"
#include "third_party/base/stl_util.h"

class CFX_ImageStretcher {
 public:
  CFX_ImageStretcher(IFX_ScanlineComposer* pDest,
                     const CFX_RetainPtr<CFX_DIBSource>& pSource,
                     int dest_width,
                     int dest_height,
                     const FX_RECT& bitmap_rect,
                     uint32_t flags);
  ~CFX_ImageStretcher();

  bool Start();
  bool Continue(IFX_Pause* pPause);

  CFX_RetainPtr<CFX_DIBSource> source() { return m_pSource; }

 private:
  bool StartQuickStretch();
  bool StartStretch();
  bool ContinueQuickStretch(IFX_Pause* pPause);
  bool ContinueStretch(IFX_Pause* pPause);

  IFX_ScanlineComposer* const m_pDest;
  CFX_RetainPtr<CFX_DIBSource> m_pSource;
  std::unique_ptr<CStretchEngine> m_pStretchEngine;
  std::unique_ptr<uint8_t, FxFreeDeleter> m_pScanline;
  std::unique_ptr<uint8_t, FxFreeDeleter> m_pMaskScanline;
  const uint32_t m_Flags;
  bool m_bFlipX;
  bool m_bFlipY;
  int m_DestWidth;
  int m_DestHeight;
  FX_RECT m_ClipRect;
  const FXDIB_Format m_DestFormat;
  const int m_DestBPP;
  int m_LineIndex;
};

#endif  // CORE_FXGE_DIB_CFX_IMAGESTRETCHER_H_
