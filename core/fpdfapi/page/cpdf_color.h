// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_COLOR_H_
#define CORE_FPDFAPI_PAGE_CPDF_COLOR_H_

#include <stdint.h>

#include <memory>
#include <optional>
#include <vector>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxge/dib/fx_dib.h"

class CPDF_ColorSpace;
class CPDF_Pattern;
class PatternValue;

class CPDF_Color {
 public:
  CPDF_Color();
  CPDF_Color(const CPDF_Color& that);

  ~CPDF_Color();

  CPDF_Color& operator=(const CPDF_Color& that);

  bool IsNull() const { return buffer_.empty() && !value_; }
  bool IsPattern() const;
  void SetColorSpace(RetainPtr<CPDF_ColorSpace> colorspace);
  void SetValueForNonPattern(std::vector<float> values);
  void SetValueForPattern(RetainPtr<CPDF_Pattern> pattern,
                          pdfium::span<float> values);

  uint32_t ComponentCount() const;
  bool IsColorSpaceRGB() const;
  bool IsColorSpaceGray() const;
  std::optional<FX_COLORREF> GetColorRef() const;

  // Should only be called if IsPattern() returns true.
  RetainPtr<CPDF_Pattern> GetPattern() const;

 protected:
  bool IsPatternInternal() const;

  std::vector<float> buffer_;            // Used for non-pattern colorspaces.
  std::unique_ptr<PatternValue> value_;  // Used for pattern colorspaces.
  RetainPtr<CPDF_ColorSpace> cs_;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_COLOR_H_
