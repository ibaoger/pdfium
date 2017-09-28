// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/gif/cfx_lzwdecompressor.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fxcodec/lbmp/fx_bmp.h"
#include "third_party/base/numerics/safe_math.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

std::unique_ptr<CFX_LZWDecompressor> CFX_LZWDecompressor::Create(
    uint8_t color_exp,
    uint8_t code_exp) {
  // color_exp generates 2^(n + 1) codes, where as the code_exp reserves 2^n.
  // This is a quirk of the GIF spec.
  if (code_exp > GIF_MAX_LZW_EXP || code_exp < color_exp + 1)
    return nullptr;
  return std::unique_ptr<CFX_LZWDecompressor>(
      new CFX_LZWDecompressor(color_exp, code_exp));
}

CFX_LZWDecompressor::CFX_LZWDecompressor(uint8_t color_exp, uint8_t code_exp)
    : code_size_(code_exp),
      code_size_cur_(0),
      code_color_end_(static_cast<uint16_t>(1 << (color_exp + 1))),
      code_clear_(static_cast<uint16_t>(1 << code_exp)),
      code_end_(static_cast<uint16_t>((1 << code_exp) + 1)),
      code_next_(0),
      code_first_(0),
      code_old_(0),
      next_in_(nullptr),
      avail_in_(0),
      bits_left_(0),
      code_store_(0) {}

CFX_LZWDecompressor::~CFX_LZWDecompressor() {}

CFX_GifDecodeStatus CFX_LZWDecompressor::Decode(uint8_t* src_buf,
                                                uint32_t src_size,
                                                uint8_t* des_buf,
                                                uint32_t* des_size) {
  if (!src_buf || src_size == 0 || !des_buf || !des_size)
    return CFX_GifDecodeStatus::Error;

  if (*des_size == 0)
    return CFX_GifDecodeStatus::InsufficientDestSize;

  next_in_ = src_buf;
  avail_in_ = src_size;

  ClearTable();

  uint32_t i = 0;
  if (decompressed_.size() != 0) {
    uint32_t extracted_size = ExtractData(des_buf, *des_size);
    if (decompressed_.size() != 0)
      return CFX_GifDecodeStatus::InsufficientDestSize;

    des_buf += extracted_size;
    i += extracted_size;
  }

  while (i <= *des_size && (avail_in_ > 0 || bits_left_ >= code_size_cur_)) {
    if (code_size_cur_ > GIF_MAX_LZW_EXP)
      return CFX_GifDecodeStatus::Error;

    if (avail_in_ > 0) {
      if (bits_left_ > 31)
        return CFX_GifDecodeStatus::Error;

      pdfium::base::CheckedNumeric<uint32_t> safe_code = *next_in_++;
      safe_code <<= bits_left_;
      safe_code |= code_store_;
      if (!safe_code.IsValid())
        return CFX_GifDecodeStatus::Error;

      code_store_ = safe_code.ValueOrDie();
      --avail_in_;
      bits_left_ += 8;
    }

    while (bits_left_ >= code_size_cur_) {
      uint16_t code =
          static_cast<uint16_t>(code_store_) & ((1 << code_size_cur_) - 1);
      code_store_ >>= code_size_cur_;
      bits_left_ -= code_size_cur_;
      if (code == code_clear_) {
        ClearTable();
        continue;
      }
      if (code == code_end_) {
        *des_size = i;
        return CFX_GifDecodeStatus::Success;
      }

      if (code_old_ != static_cast<uint16_t>(-1)) {
        if (code_next_ < GIF_MAX_LZW_CODE) {
          if (code == code_next_) {
            AddCode(code_old_, code_first_);
            if (!DecodeString(code))
              return CFX_GifDecodeStatus::Error;
          } else if (code > code_next_) {
            return CFX_GifDecodeStatus::Error;
          } else {
            if (!DecodeString(code))
              return CFX_GifDecodeStatus::Error;

            uint8_t append_char = decompressed_.front();
            AddCode(code_old_, append_char);
          }
        }
      } else {
        if (!DecodeString(code))
          return CFX_GifDecodeStatus::Error;
      }

      code_old_ = code;
      uint32_t extracted_size = ExtractData(des_buf, *des_size - i);
      if (decompressed_.size() != 0)
        return CFX_GifDecodeStatus::InsufficientDestSize;

      des_buf += extracted_size;
      i += extracted_size;
    }
  }

  if (avail_in_ != 0)
    return CFX_GifDecodeStatus::Error;

  *des_size = i;
  return CFX_GifDecodeStatus::Unfinished;
}

void CFX_LZWDecompressor::ClearTable() {
  code_size_cur_ = code_size_ + 1;
  code_next_ = code_end_ + 1;
  code_old_ = static_cast<uint16_t>(-1);
  memset(code_table_, 0, sizeof(code_table_));
  for (uint16_t i = 0; i < code_clear_; i++)
    code_table_[i].suffix = static_cast<uint8_t>(i);
  decompressed_.clear();
}

void CFX_LZWDecompressor::AddCode(uint16_t prefix_code, uint8_t append_char) {
  if (code_next_ == GIF_MAX_LZW_CODE)
    return;

  code_table_[code_next_].prefix = prefix_code;
  code_table_[code_next_].suffix = append_char;
  if (++code_next_ < GIF_MAX_LZW_CODE) {
    if (code_next_ >> code_size_cur_)
      code_size_cur_++;
  }
}

bool CFX_LZWDecompressor::DecodeString(uint16_t code) {
  decompressed_.clear();
  while (code >= code_clear_ && code <= code_next_) {
    if (code == code_table_[code].prefix ||
        decompressed_.size() >= GIF_MAX_LZW_CODE)
      return false;

    decompressed_.insert(decompressed_.begin(), code_table_[code].suffix);
    code = code_table_[code].prefix;
  }

  if (code >= code_color_end_)
    return false;

  decompressed_.insert(decompressed_.begin(), static_cast<uint8_t>(code));
  code_first_ = static_cast<uint8_t>(code);
  return true;
}

uint32_t CFX_LZWDecompressor::ExtractData(uint8_t* des_buf, uint32_t des_size) {
  if (des_size == 0)
    return 0;

  uint32_t copy_size = des_size <= decompressed_.size()
                           ? des_size
                           : static_cast<uint32_t>(decompressed_.size());
  memcpy(des_buf, decompressed_.data(), copy_size);
  decompressed_.resize(decompressed_.size() - copy_size);
  return copy_size;
}
