// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_FDE_GEDEVICE_H_
#define XFA_FDE_FDE_GEDEVICE_H_

#include <vector>

#include "core/fxge/cfx_renderdevice.h"
#include "xfa/fgas/font/cfgas_gefont.h"

class CFDE_Brush;
class CFDE_Path;
class CFDE_Pen;
class CFX_GraphStateData;

class CFDE_RenderDevice {
 public:
  CFDE_RenderDevice(CFX_RenderDevice* pDevice, bool bOwnerDevice);
  ~CFDE_RenderDevice();

  int32_t GetWidth() const;
  int32_t GetHeight() const;
  void SaveState();
  void RestoreState();
  bool SetClipPath(const CFDE_Path* pClip);
  CFDE_Path* GetClipPath() const;
  bool SetClipRect(const CFX_RectF& rtClip);
  const CFX_RectF& GetClipRect();

  FX_FLOAT GetDpiX() const;
  FX_FLOAT GetDpiY() const;

  bool DrawString(CFDE_Brush* pBrush,
                  const CFX_RetainPtr<CFGAS_GEFont>& pFont,
                  const FXTEXT_CHARPOS* pCharPos,
                  int32_t iCount,
                  FX_FLOAT fFontSize,
                  const CFX_Matrix& pMatrix);
  bool DrawPath(CFDE_Pen* pPen,
                FX_FLOAT fPenWidth,
                const CFDE_Path* pPath,
                const CFX_Matrix& pMatrix);

 private:
  bool CreatePen(CFDE_Pen* pPen,
                 FX_FLOAT fPenWidth,
                 CFX_GraphStateData& graphState);

  CFX_RenderDevice* const m_pDevice;
  CFX_RectF m_rtClip;
  bool m_bOwnerDevice;
};

#endif  // XFA_FDE_FDE_GEDEVICE_H_
