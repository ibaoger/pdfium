// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CPDF_TYPE1FONT_H_
#define CORE_FPDFAPI_FONT_CPDF_TYPE1FONT_H_

#include "core/fpdfapi/font/cpdf_simplefont.h"
#include "core/fxcrt/fx_system.h"

class CPDF_Type1Font : public CPDF_SimpleFont {
 public:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // CPDF_Font:
  bool IsStandardFont() const override;
  bool IsType1Font() const override;
  CFX_RetainPtr<CPDF_Type1Font> AsType1Font() override;

  int GlyphFromCharCodeExt(uint32_t charcode) override;

  int GetBase14Font() const { return m_Base14Font; }

 private:
  CPDF_Type1Font();
  ~CPDF_Type1Font() override;

  // CPDF_Font:
  bool Load() override;

  // CPDF_SimpleFont:
  void LoadGlyphMap() override;

#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  void SetExtGID(const char* name, int charcode);
  void CalcExtGID(int charcode);
#endif

  int m_Base14Font;
};

#endif  // CORE_FPDFAPI_FONT_CPDF_TYPE1FONT_H_
