// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCONVERT_CPDF_PSRENDERDEVICE_H_
#define CORE_FXCONVERT_CPDF_PSRENDERDEVICE_H_

#include <memory>

#include "core/fxconvert/cfx_psrenderer.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxge/ifx_renderdevicedriver.h"

class CFX_Font;
class CFX_GraphStateData;
class CFX_ImageRenderer;
class CFX_Matrix;
class CFX_PSRenderer;
class FXTEXT_CHARPOS;
class IFX_WriteStream;

class CPDF_PSRenderDevice : public IFX_RenderDeviceDriver {
 public:
  CPDF_PSRenderDevice(std::unique_ptr<IFX_WriteStream> stream,
                      int ps_level,
                      bool bCmykOutput);
  ~CPDF_PSRenderDevice() override;

  void SetHorizontalSize(int size) { m_HorzSize = size; }
  void SetVerticalSize(int size) { m_VertSize = size; }
  void SetWidth(int size) { m_Width = size; }
  void SetHeight(int size) { m_Height = size; }
  void SetBitsPerPixel(int bpp) { m_nBitsPerPixel = bpp; }

  // IFX_RenderDeviceDriver
  int GetDeviceCaps(int caps_id) const override;
  bool StartRendering() override;
  void EndRendering() override;
  void SaveState() override;
  void RestoreState(bool bKeepSaved) override;
  bool SetClip_PathFill(const CFX_PathData* pPathData,
                        const CFX_Matrix* pObject2Device,
                        int fill_mode) override;
  bool SetClip_PathStroke(const CFX_PathData* pPathData,
                          const CFX_Matrix* pObject2Device,
                          const CFX_GraphStateData* pGraphState) override;
  bool DrawPath(const CFX_PathData* pPathData,
                const CFX_Matrix* pObject2Device,
                const CFX_GraphStateData* pGraphState,
                uint32_t fill_color,
                uint32_t stroke_color,
                int fill_mode,
                int blend_type) override;
  bool GetClipBox(FX_RECT* pRect) override;
  bool SetDIBits(const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
                 uint32_t color,
                 const FX_RECT* pSrcRect,
                 int left,
                 int top,
                 int blend_type) override;
  bool StretchDIBits(const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
                     uint32_t color,
                     int dest_left,
                     int dest_top,
                     int dest_width,
                     int dest_height,
                     const FX_RECT* pClipRect,
                     uint32_t flags,
                     int blend_type) override;
  bool StartDIBits(const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
                   int bitmap_alpha,
                   uint32_t color,
                   const CFX_Matrix* pMatrix,
                   uint32_t render_flags,
                   std::unique_ptr<CFX_ImageRenderer>* handle,
                   int blend_type) override;
  bool DrawDeviceText(int nChars,
                      const FXTEXT_CHARPOS* pCharPos,
                      CFX_Font* pFont,
                      const CFX_Matrix* pObject2Device,
                      float font_size,
                      uint32_t color) override;
  void* GetPlatformSurface() const override;

 private:
  std::unique_ptr<IFX_WriteStream> m_Stream;
  bool m_bCmykOutput;
  int m_Width;
  int m_Height;
  int m_nBitsPerPixel;
  int m_HorzSize;
  int m_VertSize;
  CFX_PSRenderer m_PSRenderer;
};

#endif  // CORE_FXCONVERT_CPDF_PSRENDERDEVICE_H_
