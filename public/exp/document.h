// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef PUBLIC_EXP_DOCUMENT_H_
#define PUBLIC_EXP_DOCUMENT_H_

/**
 * This file defines the PDFium document interface. All methods related to
 * working with documents are defined here.
 */

#include "public/fpdfview.h"

#ifdef __cplusplus

#include <memory>
#include <string>

extern "C" {
#endif

typedef void* pdfium_document_t;

/**
 * Creates and returns a new empty PDF document.
 */
DLLEXPORT pdfium_document_t STDCALL pdfium_document_new();

/**
 * Cleans up the given |pdfium_document_t|. The document must not be used
 * after this method completes.
 */
DLLEXPORT void STDCALL pdfium_document_delete(pdfium_document_t);

/**
 * Load the file at |path| into a new |pdfium_document_t|. If the file is
 * encrypted the |password| will be used to decrypt the file. The |password|
 * may be NULL if the file is not encrypted. Returns the |pdfium_document_t|
 * for the file or NULL on failure. Check |pdfium_last_status| for the failure
 * reason.
 */
DLLEXPORT pdfium_document_t STDCALL pdfium_document_load(const char* path,
                                                         const char* password);

/**
 * Returns the number of pages in the |pdfium_document_t|.
 */
DLLEXPORT size_t STDCALL pdfium_document_count_pages(pdfium_document_t);

#ifdef __cplusplus
}  // extern "C"

namespace pdfium {

class Document {
 public:
  /**
   * Create a new document from the file at |path| using the given |password|
   * if the file is encrypted. Returns the |Document| on success or nullptr on
   * failure. Check |pdfium_last_status| for the failure reason.
   */
  static std::unique_ptr<Document> FromFile(const std::string& path,
                                            const std::string& password);

  /**
   * Creates a new empty PDF document.
   */
  Document();
  ~Document();

  /**
   * Returns the count of the number of pages in the document.
   */
  size_t CountPages() const;

 private:
  explicit Document(pdfium_document_t);

  pdfium_document_t doc_;
};

}  // namespace pdfium

#endif

#endif  // PUBLIC_EXP_DOCUMENT_H_
