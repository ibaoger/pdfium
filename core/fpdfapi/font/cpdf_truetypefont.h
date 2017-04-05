// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CPDF_TRUETYPEFONT_H_
#define CORE_FPDFAPI_FONT_CPDF_TRUETYPEFONT_H_

#include "core/fpdfapi/font/cpdf_simplefont.h"
#include "core/fxcrt/fx_system.h"

class CPDF_TrueTypeFont : public CPDF_SimpleFont {
 public:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // CPDF_Font:
  bool IsTrueTypeFont() const override;
  CFX_RetainPtr<CPDF_TrueTypeFont> AsTrueTypeFont() override;

 private:
  CPDF_TrueTypeFont();
  ~CPDF_TrueTypeFont() override;

  // CPDF_Font:
  bool Load() override;

  // CPDF_SimpleFont:
  void LoadGlyphMap() override;
};

#endif  // CORE_FPDFAPI_FONT_CPDF_TRUETYPEFONT_H_
