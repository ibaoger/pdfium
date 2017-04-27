// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_MOCK_IFX_RENDERDEVICE_H_
#define TESTING_MOCK_IFX_RENDERDEVICE_H_

#include <memory>

#include "core/fxge/ifx_renderdevicedriver.h"
#include "testing/gmock/include/gmock/gmock.h"

class MockIFXRenderDevice : public IFX_RenderDeviceDriver {
 public:
  MOCK_CONST_METHOD1(GetDeviceCaps, int(int caps_id));
  MOCK_METHOD0(SaveState, void());
  MOCK_METHOD1(RestoreState, void(bool bKeepSaved));
  MOCK_METHOD3(SetClip_PathFill,
               bool(const CFX_PathData* pPathData,
                    const CFX_Matrix* pObject2Device,
                    int fill_mode));
  MOCK_METHOD7(DrawPath,
               bool(const CFX_PathData* pPathData,
                    const CFX_Matrix* pObject2Device,
                    const CFX_GraphStateData* pGraphState,
                    uint32_t fill_color,
                    uint32_t stroke_color,
                    int fill_mode,
                    int blend_type));
  MOCK_METHOD1(GetClipBox, bool(FX_RECT* pRect));
  MOCK_METHOD6(SetDIBits,
               bool(const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
                    uint32_t color,
                    const FX_RECT* pSrcRect,
                    int dest_left,
                    int dest_top,
                    int blend_type));

  MOCK_METHOD7(StartDIBits,
               bool(const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
                    int bitmap_alpha,
                    uint32_t color,
                    const CFX_Matrix* pMatrix,
                    uint32_t flags,
                    std::unique_ptr<CFX_ImageRenderer>* handle,
                    int blend_type));
  MOCK_METHOD9(StretchDIBits,
               bool(const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
                    uint32_t color,
                    int dest_left,
                    int dest_top,
                    int dest_width,
                    int dest_height,
                    const FX_RECT* pClipRect,
                    uint32_t flags,
                    int blend_type));

#if 0
  virtual CFX_Matrix GetCTM() const;
  virtual bool StartRendering();
  virtual void EndRendering();
  virtual bool SetClip_PathStroke(const CFX_PathData* pPathData,
                                  const CFX_Matrix* pObject2Device,
                                  const CFX_GraphStateData* pGraphState);
  virtual bool SetPixel(int x, int y, uint32_t color);
  virtual bool FillRectWithBlend(const FX_RECT* pRect,
                                 uint32_t fill_color,
                                 int blend_type);
  virtual bool DrawCosmeticLine(float x1,
                                float y1,
                                float x2,
                                float y2,
                                uint32_t color,
                                int blend_type);
  virtual bool GetDIBits(const CFX_RetainPtr<CFX_DIBitmap>& pBitmap,
                         int left,
                         int top);
  virtual CFX_RetainPtr<CFX_DIBitmap> GetBackDrop();
  virtual bool ContinueDIBits(CFX_ImageRenderer* handle, IFX_Pause* pPause);
  virtual bool DrawDeviceText(int nChars,
                              const FXTEXT_CHARPOS* pCharPos,
                              CFX_Font* pFont,
                              const CFX_Matrix* pObject2Device,
                              float font_size,
                              uint32_t color);
  virtual void* GetPlatformSurface() const;
  virtual int GetDriverType() const;
  virtual void ClearDriver();
  virtual bool DrawShading(const CPDF_ShadingPattern* pPattern,
                           const CFX_Matrix* pMatrix,
                           const FX_RECT& clip_rect,
                           int alpha,
                           bool bAlphaMode);
  virtual bool SetBitsWithMask(const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
                               const CFX_RetainPtr<CFX_DIBSource>& pMask,
                               int left,
                               int top,
                               int bitmap_alpha,
                               int blend_type);
#if defined _SKIA_SUPPORT_ || defined _SKIA_SUPPORT_PATHS_
  virtual void Flush();
#endif
#endif  // 0
};

#endif  // TESTING_MOCK_IFX_RENDERDEVICE_H_
