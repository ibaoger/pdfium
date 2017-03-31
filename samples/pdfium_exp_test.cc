// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/exp/pdfium.h"
#include "third_party/base/ptr_util.h"

int main(int argc, const char* argv[]) {
  if (argc < 2)
    return -1;

  pdfium_config_t config{1, nullptr, nullptr, 0};
  pdfium_initialize(&config);

  // C API
  for (int i = 1; i < argc; ++i) {
    pdfium_document_t doc = pdfium_document_new();
    pdfium_document_status_t ret = pdfium_document_load(doc, argv[i], nullptr);
    if (ret != PDFIUM_STATUS_SUCCESS) {
      printf("Failed to load %s: %u\n", argv[i], ret);
      continue;
    }

    printf("%s has %zu pages\n", argv[i], pdfium_document_count_pages(doc));
    pdfium_document_delete(doc);
  }

  // C++ API
  for (int i = 1; i < argc; ++i) {
    auto doc = pdfium::MakeUnique<pdfium::Document>();
    pdfium_document_status_t ret = doc->Load(argv[i], "");
    if (ret != PDFIUM_STATUS_SUCCESS) {
      printf("Failed to load %s: %u\n", argv[i], ret);
      continue;
    }

    printf("%s has %zu pages\n", argv[i], doc->CountPages());
  }

  pdfium_shutdown();
}
