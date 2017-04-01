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

  for (int i = 1; i < argc; ++i) {
    auto doc = pdfium::Document::FromFile(argv[i], "");
    if (!doc) {
      printf("Failed to load %s: %s\n", argv[i],
             pdfium_error_str(pdfium_last_status()));
      continue;
    }
    printf("%s has %zu pages\n", argv[i], doc->CountPages());
  }

  pdfium_shutdown();
}
