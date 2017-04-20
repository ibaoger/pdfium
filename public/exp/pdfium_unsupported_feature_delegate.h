// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PUBLIC_EXP_PDFIUM_UNSUPPORTED_FEATURE_DELEGATE_H_
#define PUBLIC_EXP_PDFIUM_UNSUPPORTED_FEATURE_DELEGATE_H_

namespace pdfium {

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

}  // namespace pdfium

#endif  // PUBLIC_EXP_PDFIUM_UNSUPPORTED_FEATURE_DELEGATE_H_
