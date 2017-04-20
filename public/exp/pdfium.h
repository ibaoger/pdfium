// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PUBLIC_EXP_PDFIUM_H_
#define PUBLIC_EXP_PDFIUM_H_

#include <memory>
#include <string>
#include <vector>

// NOLINTNEXTLINE(build/include)
#include "pdfium_status.h"

// This file is the main include for the PDFium API.

namespace pdfium {

class PDFiumConfig {
 public:
  PDFiumConfig();
  ~PDFiumConfig();

  std::vector<std::string> font_paths;

  uint32_t embedder_slot;
  void* isolate;

 private:
  PDFiumConfig(const PDFiumConfig&) = delete;
};

class UnsupportedFeatureDelegate {
 public:
  enum class Type {
    kXFAForm = 1,
    kProtableCollection = 2,
    kAttachment = 3,
    kSecurity = 4,
    kSharedReview = 5,
    kSharedFormAcrobat = 6,
    kSharedFormFilesystem = 7,
    kSharedFormEmail = 8,
    kAnnotation3D = 11,
    kAnnotationMovie = 12,
    kAnnotationSound = 13,
    kAnnotationScreenMedia = 14,
    kAnnotationRichMedia = 15,
    kAnnotationAttachment = 16,
    kAnnotationSignature = 17
  };

  UnsupportedFeatureDelegate();
  virtual ~UnsupportedFeatureDelegate();

  virtual void HandleUnsupportedFeature(Type) const;
};

void Initialize(std::unique_ptr<PDFiumConfig> cfg);
void SetUnsupportedFeatureDelegate(
    std::unique_ptr<UnsupportedFeatureDelegate> delgate);
void Shutdown();

}  // namespace pdfium

#endif  // PUBLIC_EXP_PDFIUM_H_
