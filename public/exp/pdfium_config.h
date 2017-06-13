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

  void SetIsolate(void* isolate) { isolate_ = isolate; }
  void* GetIsolate() const { return isolate_; }

  void SetEmbedderSlot(uint32_t slot) { embedder_slot_ = slot; }
  uint32_t GetEmbedderSlot() const { return embedder_slot_; }

  void SetFontPaths(const std::vector<std::string>& paths) {
    font_paths_ = paths:
  }
  const std::vector<std::string>& GetFontPaths() const { return font_paths_; }

 private:
  PDFiumConfig(const PDFiumConfig&) = delete;

  std::vector<std::string> font_paths_;
  uint32_t embedder_slot_;
  void* isolate_;
};

}  // namespace pdfium

#endif  // PUBLIC_EXP_PDFIUM_CONFIG_H_
