// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PUBLIC_EXP_PDFIUM_DOCUMENT_H_
#define PUBLIC_EXP_PDFIUM_DOCUMENT_H_

#include <memory>
#include <string>
#include <utility>

// NOLINTNEXTLINE(build/include)
#include "pdfium_status.h"
#include "public/cpp/fpdf_deleters.h"
#include "public/fpdfview.h"

namespace pdfium {

class Document {
 public:
  static std::pair<std::unique_ptr<Document>, Status> LoadFile(
      const std::string& file,
      const std::string& password);
  static std::pair<std::unique_ptr<Document>, Status>
  LoadFromMemory(const void* data, size_t size, const std::string& password);

  Document();
  // Deprecated. This will go away when the old API goes away.
  explicit Document(FPDF_DOCUMENT doc);
  ~Document();

  size_t PageCount() const;

 private:
  Document(const Document&) = delete;
  Document operator=(const Document&) = delete;

  // TOOD(dsinclair): This should go away when the old API is removed.
  std::unique_ptr<void, FPDFDocumentDeleter> doc_;
};

}  // namespace pdfium

#endif  // PUBLIC_EXP_PDFIUM_DOCUMENT_H_
