// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/exp/pdfium.h"
#include "public/fpdfview.h"

namespace {

FPDF_LIBRARY_CONFIG g_config_;

}  // namespace

void pdfium_initialize(pdfium_config_t* config) {
  g_config_.version = config->version;
  g_config_.m_pUserFontPaths = config->user_font_paths;
  g_config_.m_pIsolate = config->js_isolate;
  g_config_.m_v8EmbedderSlot = config->js_embedder_slot;

  FPDF_InitLibraryWithConfig(&g_config_);
}

void pdfium_shutdown() {
  FPDF_DestroyLibrary();

  g_config_.version = 0;
  g_config_.m_pUserFontPaths = nullptr;
  g_config_.m_pIsolate = nullptr;
  g_config_.m_v8EmbedderSlot = 0;
}

pdfium_status_t pdfium_last_status() {
  printf("%lu\n", FPDF_GetLastError());
  return static_cast<pdfium_status_t>(FPDF_GetLastError());
}

const char* pdfium_error_str(pdfium_status_t err) {
  switch (err) {
    case PDFIUM_STATUS_SUCCESS:
      return "Success";
    case PDFIUM_STATUS_UNKNOWN:
      return "Unknown";
    case PDFIUM_STATUS_FILE_ERROR:
      return "File error";
    case PDFIUM_STATUS_FORMAT_ERROR:
      return "Format error";
    case PDFIUM_STATUS_PASSWORD_ERROR:
      return "Password error";
    case PDFIUM_STATUS_SECURITY_ERROR:
      return "Security error";
    case PDFIUM_STATUS_PAGE_NOT_FOUND_OR_CONTENT_ERROR:
      return "Page not found, or content error";

#ifdef PDF_ENABLE_XFA
    case PDFIUM_STATUS_XFA_LOAD_ERROR:
      return "XFA load error";
    case PDFIUM_STATUS_XFA_LAYOUT_ERROR:
      return "XFA layout error";
#endif  // PDF_ENABLE_XFA
  }
  return "Unknown error code";
}
