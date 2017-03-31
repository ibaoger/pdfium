// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef PUBLIC_EXP_DOCUMENT_H_
#define PUBLIC_EXP_DOCUMENT_H_

#include "public/fpdfview.h"

#ifdef __cplusplus

#include <memory>
#include <string>

extern "C" {
#endif

typedef void* pdfium_document_t;

DLLEXPORT pdfium_document_t STDCALL pdfium_document_new();

DLLEXPORT void STDCALL pdfium_document_delete(pdfium_document_t);

DLLEXPORT pdfium_document_t STDCALL pdfium_document_load(const char* path,
                                                         const char* password);

DLLEXPORT size_t STDCALL pdfium_document_count_pages(pdfium_document_t);

#ifdef __cplusplus
}  // extern "C"

namespace pdfium {

class Document {
 public:
  static std::unique_ptr<Document> FromFile(const std::string& path,
                                            const std::string& password);

  Document();
  ~Document();

  size_t CountPages() const;

 private:
  explicit Document(pdfium_document_t);

  pdfium_document_t doc_;
};

}  // namespace pdfium

#endif

#endif  // PUBLIC_EXP_DOCUMENT_H_
