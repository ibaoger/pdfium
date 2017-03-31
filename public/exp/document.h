// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef PUBLIC_EXP_DOCUMENT_H_
#define PUBLIC_EXP_DOCUMENT_H_

#include "public/fpdfview.h"

#ifdef __cplusplus

#include <string>

extern "C" {
#endif

enum pdfium_document_status_t {
  PDFIUM_STATUS_SUCCESS = 0,
  PDFIUM_STATUS_UNKNOWN = 1,
  PDFIUM_STATUS_FILE_ERROR = 2,
  PDFIUM_STATUS_FORMAT_ERROR = 3,
  PDFIUM_STATUS_PASSWORD_ERROR = 4,
  PDFIUM_STATUS_SECURITY_ERROR = 5
};

typedef void* pdfium_document_t;

DLLEXPORT pdfium_document_t STDCALL pdfium_document_new();

DLLEXPORT void STDCALL pdfium_document_delete(pdfium_document_t doc);

DLLEXPORT pdfium_document_status_t STDCALL
pdfium_document_load(pdfium_document_t doc,
                     const char* path,
                     const char* password);

DLLEXPORT size_t STDCALL pdfium_document_count_pages(pdfium_document_t doc);

#ifdef __cplusplus
}  // extern "C"

namespace pdfium {

class Document {
 public:
  Document();
  ~Document();

  pdfium_document_status_t Load(const std::string& path,
                                const std::string& password);

  size_t CountPages() const;

 private:
  FPDF_DOCUMENT doc_;
};

}  // namespace pdfium

#endif

#endif  // PUBLIC_EXP_DOCUMENT_H_
