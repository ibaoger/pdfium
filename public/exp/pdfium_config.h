// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PUBLIC_EXP_PDFIUM_CONFIG_H_
#define PUBLIC_EXP_PDFIUM_CONFIG_H_

#include <string>
#include <vector>

namespace pdfium {

class PDFiumConfig {
 public:
  PDFiumConfig();
  ~PDFiumConfig();

  std::vector<std::string> font_paths;

  uint32_t embedder_slot;
  void* isolate;

 private:
  PDFiumConfig(const PDFiumConfig&) = delete;
};

}  // namespace pdfium

#endif  // PUBLIC_EXP_PDFIUM_CONFIG_H_
