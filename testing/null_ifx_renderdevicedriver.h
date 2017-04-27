// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_NULL_IFX_RENDERDEVICEDRIVER_H_
#define TESTING_NULL_IFX_RENDERDEVICEDRIVER_H_

#include <memory>

#include "core/fxge/ifx_renderdevicedriver.h"

class NullIFXRenderDeviceDriver : public IFX_RenderDeviceDriver {
 public:
  int GetDeviceCaps(int caps_id) const override { return 0; }
  void SaveState() override {}
  void RestoreState(bool bKeepSaved) override{};
  bool SetClip_PathFill(const CFX_PathData* pPathData,
                        const CFX_Matrix* pObject2Device,
                        int fill_mode) override {
    return false;
  };
  bool DrawPath(const CFX_PathData* pPathData,
                const CFX_Matrix* pObject2Device,
                const CFX_GraphStateData* pGraphState,
                uint32_t fill_color,
                uint32_t stroke_color,
                int fill_mode,
                int blend_type) override {
    return false;
  }
  bool GetClipBox(FX_RECT* pRect) override { return false; }
  bool SetDIBits(const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
                 uint32_t color,
                 const FX_RECT* pSrcRect,
                 int dest_left,
                 int dest_top,
                 int blend_type) override {
    return false;
  }
  bool StartDIBits(const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
                   int bitmap_alpha,
                   uint32_t color,
                   const CFX_Matrix* pMatrix,
                   uint32_t flags,
                   std::unique_ptr<CFX_ImageRenderer>* handle,
                   int blend_type) override {
    return false;
  }
  bool StretchDIBits(const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
                     uint32_t color,
                     int dest_left,
                     int dest_top,
                     int dest_width,
                     int dest_height,
                     const FX_RECT* pClipRect,
                     uint32_t flags,
                     int blend_type) override {
    return false;
  }
};

#endif  // TESTING_NULL_IFX_RENDERDEVICEDRIVER_H_
