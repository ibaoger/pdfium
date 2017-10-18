// Copyright 2017 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "core/fxcodec/gif/cfx_lzwdecompressor.h"
#include "third_party/base/numerics/safe_conversions.h"

// Between 2x and 5x is a standard range for LZW according to a quick
// search of papers. Running up to 10x to catch any niche cases.
const uint32_t kMaxCompressionRatio = 10;

class LZWFuzzer {
 public:
  LZWFuzzer(const uint8_t* data, size_t size) : data_(data), data_size_(size) {}

  void Fuzz(uint8_t color_exp, uint8_t code_exp) {
    std::unique_ptr<CFX_LZWDecompressor> decompressor =
        CFX_LZWDecompressor::Create(color_exp, code_exp);
    if (!decompressor)
      return;

    // Check that there isn't going to be an overflow in the destination buffer
    // size.
    if (data_size_ >
        std::numeric_limits<uint32_t>::max() / kMaxCompressionRatio)
      return;

    uint8_t* src_buf = const_cast<uint8_t*>(data_);
    uint32_t src_size = static_cast<uint32_t>(data_size_);

    for (uint32_t compressions_ratio = 2; compressions_ratio <= 10;
         compressions_ratio++) {
      std::vector<uint8_t> des_buf(compressions_ratio * data_size_);
      uint32_t des_size = static_cast<uint32_t>(des_buf.size());
      if (CFX_GifDecodeStatus::InsufficientDestSize !=
          decompressor->Decode(src_buf, src_size, des_buf.data(), &des_size))
        return;
    }
  }

 protected:
  const uint8_t* data_;
  const size_t data_size_;
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  LZWFuzzer fuzzer(data, size);

  // Normally the GIF would provide the code and color sizes, so just
  // going to try all the possible options. Bad combos will either
  // cause an decompression to fail outright or 'succeed' but the
  // output be garbage.
  for (uint8_t code_exp = 0; code_exp <= GIF_MAX_LZW_EXP; code_exp++) {
    for (uint8_t color_exp = 0; color_exp < code_exp; color_exp++) {
      fuzzer.Fuzz(color_exp, code_exp);
    }
  }

  return 0;
}
