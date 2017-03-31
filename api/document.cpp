// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/exp/document.h"

#include <string>

namespace {

pdfium::Document* ToDocument(pdfium_document_t doc) {
  return static_cast<pdfium::Document*>(doc);
}

pdfium_document_t FromDocument(pdfium::Document* doc) {
  return static_cast<pdfium_document_t>(doc);
}

}  // namespace

pdfium_document_t pdfium_document_new() {
  return FromDocument(new pdfium::Document());
}

void pdfium_document_delete(pdfium_document_t doc) {
  if (!doc)
    return;

  delete ToDocument(doc);
}

pdfium_document_status_t pdfium_document_load(pdfium_document_t doc,
                                              const char* path,
                                              const char* password) {
  if (!doc)
    return PDFIUM_STATUS_UNKNOWN;
  if (path == nullptr)
    return PDFIUM_STATUS_FILE_ERROR;

  pdfium::Document* document = ToDocument(doc);
  std::string pass = password ? std::string(password) : std::string();
  return document->Load(path, pass);
}

DLLEXPORT size_t STDCALL pdfium_document_count_pages(pdfium_document_t doc) {
  if (!doc)
    return 0;

  return ToDocument(doc)->CountPages();
}

namespace pdfium {

Document::Document() : doc_(nullptr) {}

Document::~Document() {
  FPDF_CloseDocument(doc_);
}

pdfium_document_status_t Document::Load(const std::string& path,
                                        const std::string& password) {
  doc_ = FPDF_LoadDocument(path.c_str(), password.c_str());
  if (doc_)
    return PDFIUM_STATUS_SUCCESS;

  unsigned long err = FPDF_GetLastError();
  if (err > 5)
    return PDFIUM_STATUS_UNKNOWN;

  return static_cast<pdfium_document_status_t>(err);
}

size_t Document::CountPages() const {
  return FPDF_GetPageCount(doc_);
}

}  // namespace pdfium
