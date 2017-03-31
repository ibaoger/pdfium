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
