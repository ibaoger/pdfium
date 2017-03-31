// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/exp/document.h"

#include <memory>
#include <string>

#include "public/fpdf_edit.h"

namespace {

FPDF_DOCUMENT ToFPDFDoc(pdfium_document_t doc) {
  return static_cast<FPDF_DOCUMENT>(doc);
}

pdfium_document_t FromFPDFDoc(FPDF_DOCUMENT doc) {
  return static_cast<pdfium_document_t>(doc);
}

}  // namespace

pdfium_document_t pdfium_document_new() {
  return FromFPDFDoc(FPDF_CreateNewDocument());
}

void pdfium_document_delete(pdfium_document_t doc) {
  if (!doc)
    return;
  FPDF_CloseDocument(doc);
}

pdfium_document_t pdfium_document_load(const char* path, const char* password) {
  return path ? FromFPDFDoc(FPDF_LoadDocument(path, password)) : nullptr;
}

DLLEXPORT size_t STDCALL pdfium_document_count_pages(pdfium_document_t doc) {
  return doc ? FPDF_GetPageCount(ToFPDFDoc(doc)) : 0;
}

namespace pdfium {

// static
std::unique_ptr<Document> Document::FromFile(const std::string& path,
                                             const std::string& password) {
  pdfium_document_t doc = pdfium_document_load(path.c_str(), password.c_str());
  return doc ? std::unique_ptr<Document>(new Document(doc)) : nullptr;
}

Document::Document() : doc_(pdfium_document_new()) {}

Document::Document(pdfium_document_t doc) : doc_(doc) {}

Document::~Document() {
  pdfium_document_delete(doc_);
}

size_t Document::CountPages() const {
  return pdfium_document_count_pages(doc_);
}

}  // namespace pdfium
