// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxconvert/cpdf_psrenderdevice.h"

#include <utility>

#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/dib/cfx_imagerenderer.h"

CPDF_PSRenderDevice::CPDF_PSRenderDevice(CFX_RetainPtr<IFX_WriteStream> stream,
                                         int pslevel,
                                         bool bCmykOutput)
    : m_Stream(stream),
      m_bCmykOutput(bCmykOutput),
      m_Width(0),
      m_Height(0),
      m_nBitsPerPixel(0),
      m_HorzSize(0),
      m_VertSize(0),
      m_PSRenderer(m_Stream.Get(), pslevel, m_Width, m_Height, bCmykOutput) {}

CPDF_PSRenderDevice::~CPDF_PSRenderDevice() {
  EndRendering();
}

int CPDF_PSRenderDevice::GetDeviceCaps(int caps_id) const {
  switch (caps_id) {
    case FXDC_DEVICE_CLASS:
      return FXDC_PRINTER;
    case FXDC_PIXEL_WIDTH:
      return m_Width;
    case FXDC_PIXEL_HEIGHT:
      return m_Height;
    case FXDC_BITS_PIXEL:
      return m_nBitsPerPixel;
    case FXDC_RENDER_CAPS:
      return m_bCmykOutput ? FXRC_BIT_MASK | FXRC_CMYK_OUTPUT : FXRC_BIT_MASK;
    case FXDC_HORZ_SIZE:
      return m_HorzSize;
    case FXDC_VERT_SIZE:
      return m_VertSize;
  }
  return 0;
}

bool CPDF_PSRenderDevice::StartRendering() {
  m_PSRenderer.StartRendering();
  return true;
}

void CPDF_PSRenderDevice::EndRendering() {
  m_PSRenderer.EndRendering();
}

void CPDF_PSRenderDevice::SaveState() {
  m_PSRenderer.SaveState();
}

void CPDF_PSRenderDevice::RestoreState(bool bKeepSaved) {
  m_PSRenderer.RestoreState(bKeepSaved);
}

bool CPDF_PSRenderDevice::SetClip_PathFill(const CFX_PathData* pPathData,
                                           const CFX_Matrix* pObject2Device,
                                           int fill_mode) {
  m_PSRenderer.SetClip_PathFill(pPathData, pObject2Device, fill_mode);
  return true;
}

bool CPDF_PSRenderDevice::SetClip_PathStroke(
    const CFX_PathData* pPathData,
    const CFX_Matrix* pObject2Device,
    const CFX_GraphStateData* pGraphState) {
  m_PSRenderer.SetClip_PathStroke(pPathData, pObject2Device, pGraphState);
  return true;
}

bool CPDF_PSRenderDevice::DrawPath(const CFX_PathData* pPathData,
                                   const CFX_Matrix* pObject2Device,
                                   const CFX_GraphStateData* pGraphState,
                                   FX_ARGB fill_color,
                                   FX_ARGB stroke_color,
                                   int fill_mode,
                                   int blend_type) {
  if (blend_type != FXDIB_BLEND_NORMAL) {
    return false;
  }
  return m_PSRenderer.DrawPath(pPathData, pObject2Device, pGraphState,
                               fill_color, stroke_color, fill_mode & 3);
}

bool CPDF_PSRenderDevice::GetClipBox(FX_RECT* pRect) {
  *pRect = m_PSRenderer.GetClipBox();
  return true;
}

bool CPDF_PSRenderDevice::SetDIBits(const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
                                    uint32_t color,
                                    const FX_RECT* pSrcRect,
                                    int left,
                                    int top,
                                    int blend_type) {
  if (blend_type != FXDIB_BLEND_NORMAL)
    return false;
  return m_PSRenderer.SetDIBits(pBitmap, color, left, top);
}

bool CPDF_PSRenderDevice::StretchDIBits(
    const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
    uint32_t color,
    int dest_left,
    int dest_top,
    int dest_width,
    int dest_height,
    const FX_RECT* pClipRect,
    uint32_t flags,
    int blend_type) {
  if (blend_type != FXDIB_BLEND_NORMAL)
    return false;
  return m_PSRenderer.StretchDIBits(pBitmap, color, dest_left, dest_top,
                                    dest_width, dest_height, flags);
}

bool CPDF_PSRenderDevice::StartDIBits(
    const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
    int bitmap_alpha,
    uint32_t color,
    const CFX_Matrix* pMatrix,
    uint32_t render_flags,
    std::unique_ptr<CFX_ImageRenderer>* handle,
    int blend_type) {
  if (blend_type != FXDIB_BLEND_NORMAL)
    return false;

  if (bitmap_alpha < 255)
    return false;

  *handle = nullptr;
  return m_PSRenderer.DrawDIBits(pBitmap, color, pMatrix, render_flags);
}

bool CPDF_PSRenderDevice::DrawDeviceText(int nChars,
                                         const FXTEXT_CHARPOS* pCharPos,
                                         CFX_Font* pFont,
                                         const CFX_Matrix* pObject2Device,
                                         float font_size,
                                         uint32_t color) {
  return m_PSRenderer.DrawText(nChars, pCharPos, pFont, pObject2Device,
                               font_size, color);
}

void* CPDF_PSRenderDevice::GetPlatformSurface() const {
  return m_Stream.Get();
}
