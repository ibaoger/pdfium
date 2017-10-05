// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_RENDEROPTIONS_H_
#define CORE_FPDFAPI_RENDER_CPDF_RENDEROPTIONS_H_

#include "core/fpdfdoc/cpdf_occontext.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/fx_dib.h"

#define RENDER_CLEARTYPE 0x00000001
#define RENDER_PRINTGRAPHICTEXT 0x00000002
#define RENDER_FORCE_DOWNSAMPLE 0x00000004
#define RENDER_PRINTPREVIEW 0x00000008
#define RENDER_BGR_STRIPE 0x00000010
#define RENDER_NO_NATIVETEXT 0x00000020
#define RENDER_FORCE_HALFTONE 0x00000040
#define RENDER_RECT_AA 0x00000080
#define RENDER_FILL_FULLCOVER 0x00000100
#define RENDER_PRINTIMAGETEXT 0x00000200
#define RENDER_OVERPRINT 0x00000400
#define RENDER_THINLINE 0x00000800
#define RENDER_BREAKFORMASKS 0x00001000
#define RENDER_NOTEXTSMOOTH 0x10000000
#define RENDER_NOPATHSMOOTH 0x20000000
#define RENDER_NOIMAGESMOOTH 0x40000000
#define RENDER_LIMITEDIMAGECACHE 0x80000000

class CPDF_RenderOptions {
 public:
  enum Type { kNormal = 0, kGray, kAlpha };

  CPDF_RenderOptions();
  CPDF_RenderOptions(const CPDF_RenderOptions& rhs);
  ~CPDF_RenderOptions();

  FX_ARGB TranslateColor(FX_ARGB argb) const;

  void SetColorMode(Type mode) { m_ColorMode = mode; }
  bool ColorModeIs(Type mode) const { return m_ColorMode == mode; }

  bool HasFlag(uint32_t flag) const { return !!(m_Flags & flag); }
  uint32_t GetFlags() const { return m_Flags; }
  void SetFlags(uint32_t flags) { m_Flags = flags; }

  uint32_t GetCacheSizeLimit() const { return m_dwLimitCacheSize; }

  void SetDrawAnnots(bool draw) { m_bDrawAnnots = draw; }
  bool GetDrawAnnots() const { return m_bDrawAnnots; }

  void SetOCContext(RetainPtr<CPDF_OCContext> context) {
    m_pOCContext = context;
  }
  CPDF_OCContext* GetOCContext() const { return m_pOCContext.Get(); }

 private:
  Type m_ColorMode;
  uint32_t m_Flags;
  uint32_t m_dwLimitCacheSize;
  bool m_bDrawAnnots;
  RetainPtr<CPDF_OCContext> m_pOCContext;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_RENDEROPTIONS_H_
