// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PUBLIC_EXP_PDFIUM_STATUS_H_
#define PUBLIC_EXP_PDFIUM_STATUS_H_

namespace pdfium {

enum class Status {
  kSuccess = 0,
  kUnknown,
  kFile,
  kFormat,
  kPassword,
  kSecurity,
  kPage,
#ifdef PDF_ENABLE_XFA
  kXFALoad,
  kXFALayout,
#endif
};

}  // namespace pdfium

#endif  // PUBLIC_EXP_PDFIUM_STATUS_H_
