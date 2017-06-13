// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/exp/pdfium_document.h"

#include <limits>
#include <memory>
#include <string>
#include <utility>

#include "public/fpdf_edit.h"
#include "third_party/base/ptr_util.h"

namespace pdfium {

// static
std::pair<std::unique_ptr<Document>, Status> Document::LoadFile(
    const std::string& file,
    const std::string& password) {
  std::unique_ptr<void, FPDFDocumentDeleter> doc(
      FPDF_LoadDocument(file.c_str(), password.c_str()));
  int err = FPDF_GetLastError();

  // Note, we can get a null document, but still have a SUCCESS error code.
  // We need to convert that SUCCESS to a failure if we don't have a doc.
  if (!doc || err != FPDF_ERR_SUCCESS) {
    // TODO(dsinclair): This should probably be something better then unknown.
    return {nullptr, err == FPDF_ERR_SUCCESS ? Status::kUnknown
                                             : static_cast<Status>(err)};
  }

  return {pdfium::MakeUnique<Document>(doc.release()), Status::kSuccess};
}

// static
std::pair<std::unique_ptr<Document>, Status> Document::LoadFromMemory(
    const void* data,
    size_t size,
    const std::string& password) {
  if (size > std::numeric_limits<int>::max()) {
    // TODO(dsinclair): File size error?
    return {nullptr, Status::kFormat};
  }
  std::unique_ptr<void, FPDFDocumentDeleter> doc(
      FPDF_LoadMemDocument(data, static_cast<int>(size), password.c_str()));
  int err = FPDF_GetLastError();

  // Note, we can get a null document, but still have a SUCCESS error code.
  // We need to convert that SUCCESS to a failure if we don't have a doc.
  if (!doc || err != FPDF_ERR_SUCCESS) {
    // TODO(dsinclair): This should probably be something better then unknown.
    return {nullptr, err == FPDF_ERR_SUCCESS ? Status::kUnknown
                                             : static_cast<Status>(err)};
  }

  return {pdfium::MakeUnique<Document>(doc.release()), Status::kSuccess};
}

// TODO(dsinclair): Make sure FPDF_CreateNewDocument can't return nullptr.
Document::Document() : doc_(FPDF_CreateNewDocument()) {}

Document::Document(FPDF_DOCUMENT doc) : doc_(doc) {}

Document::~Document() {}

size_t Document::PageCount() const {
  return FPDF_GetPageCount(doc_.get());
}

}  // namespace pdfium
