// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_ANDROID_CFPF_SKIAPATHFONT_H_
#define CORE_FXGE_ANDROID_CFPF_SKIAPATHFONT_H_

#include "core/fxcrt/fx_system.h"
#include "core/fxge/android/cfpf_skiafontdescriptor.h"

#define FPF_SKIAFONTTYPE_Path 1

class CFPF_SkiaPathFont : public CFPF_SkiaFontDescriptor {
 public:
  CFPF_SkiaPathFont();
  ~CFPF_SkiaPathFont() override;

  // CFPF_SkiaFontDescriptor
  int32_t GetType() const override;

  void SetPath(const char* pPath);

  char* m_pPath;
};

#endif  // CORE_FXGE_ANDROID_CFPF_SKIAPATHFONT_H_
