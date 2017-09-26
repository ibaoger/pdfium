// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/lgif/cfx_lzwdecoder.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fxcodec/lbmp/fx_bmp.h"
#include "third_party/base/numerics/safe_math.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

std::unique_ptr<CFX_LZWDecoder> CFX_LZWDecoder::MakeDecoder(char* error_ptr,
                                                            uint8_t color_exp,
                                                            uint8_t code_exp) {
  if (!error_ptr || code_exp > GIF_MAX_LZW_EXP || code_exp + 1 < color_exp)
    return nullptr;
  return pdfium::MakeUnique<CFX_LZWDecoder>(error_ptr, color_exp, code_exp);
}

CFX_LZWDecoder::CFX_LZWDecoder(char* error_ptr,
                               uint8_t color_exp,
                               uint8_t code_exp)
    : code_size_(code_exp),
      code_size_cur_(0),
      code_color_end_(static_cast<uint16_t>(2 << color_exp)),
      code_clear_(static_cast<uint16_t>(1 << code_exp)),
      code_end_(static_cast<uint16_t>((1 << code_exp) + 1)),
      code_next_(0),
      code_first_(0),
      stack_size_(0),
      code_old_(0),
      next_in_(nullptr),
      avail_in_(0),
      bits_left_(0),
      code_store_(0),
      err_msg_(error_ptr) {}

CFX_LZWDecoder::~CFX_LZWDecoder() {}

GifDecodeStatus CFX_LZWDecoder::Decode(uint8_t* src_buf,
                                       uint32_t src_size,
                                       uint8_t* des_buf,
                                       uint32_t* des_size) {
  if (!src_buf || src_size == 0 || !des_buf || !des_size) {
    strncpy(err_msg_, "Invalid inputs", GIF_MAX_ERROR_SIZE - 1);
    return GifDecodeStatus::Error;
  }

  if (*des_size == 0)
    return GifDecodeStatus::InsufficientDestSize;

  next_in_ = src_buf;
  avail_in_ = src_size;

  ClearTable();

  uint32_t i = 0;
  if (stack_size_ != 0) {
    if (*des_size < stack_size_) {
      memcpy(des_buf, &stack_[GIF_MAX_LZW_CODE - stack_size_], *des_size);
      stack_size_ -= static_cast<uint16_t>(*des_size);
      return GifDecodeStatus::InsufficientDestSize;
    }
    memcpy(des_buf, &stack_[GIF_MAX_LZW_CODE - stack_size_], stack_size_);
    des_buf += stack_size_;
    i += stack_size_;
    stack_size_ = 0;
  }

  while (i <= *des_size && (avail_in_ > 0 || bits_left_ >= code_size_cur_)) {
    if (code_size_cur_ > GIF_MAX_LZW_EXP) {
      strncpy(err_msg_, "Code Length Out Of Range", GIF_MAX_ERROR_SIZE - 1);
      return GifDecodeStatus::Error;
    }
    if (avail_in_ > 0) {
      if (bits_left_ > 31) {
        strncpy(err_msg_, "Decode Error", GIF_MAX_ERROR_SIZE - 1);
        return GifDecodeStatus::Error;
      }
      pdfium::base::CheckedNumeric<uint32_t> safe_code = *next_in_++;
      safe_code <<= bits_left_;
      safe_code |= code_store_;
      if (!safe_code.IsValid()) {
        strncpy(err_msg_, "Code Store Out Of Range", GIF_MAX_ERROR_SIZE - 1);
        return GifDecodeStatus::Error;
      }
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
        return GifDecodeStatus::Success;
      }
      if (code_old_ != static_cast<uint16_t>(-1)) {
        if (code_next_ < GIF_MAX_LZW_CODE) {
          if (code == code_next_) {
            AddCode(code_old_, code_first_);
            if (!DecodeString(code)) {
              strncpy(err_msg_, "String Decoding Error",
                      GIF_MAX_ERROR_SIZE - 1);
              return GifDecodeStatus::Error;
            }
          } else if (code > code_next_) {
            strncpy(err_msg_, "Decode Error, Out Of Range",
                    GIF_MAX_ERROR_SIZE - 1);
            return GifDecodeStatus::Error;
          } else {
            if (!DecodeString(code)) {
              strncpy(err_msg_, "String Decoding Error",
                      GIF_MAX_ERROR_SIZE - 1);
              return GifDecodeStatus::Error;
            }
            uint8_t append_char = stack_[GIF_MAX_LZW_CODE - stack_size_];
            AddCode(code_old_, append_char);
          }
        }
      } else {
        if (!DecodeString(code)) {
          strncpy(err_msg_, "String Decoding Error", GIF_MAX_ERROR_SIZE - 1);
          return GifDecodeStatus::Error;
        }
      }
      code_old_ = code;
      if (i + stack_size_ > *des_size) {
        memcpy(des_buf, &stack_[GIF_MAX_LZW_CODE - stack_size_], *des_size - i);
        stack_size_ -= static_cast<uint16_t>(*des_size - i);
        return GifDecodeStatus::InsufficientDestSize;
      }
      memcpy(des_buf, &stack_[GIF_MAX_LZW_CODE - stack_size_], stack_size_);
      des_buf += stack_size_;
      i += stack_size_;
      stack_size_ = 0;
    }
  }
  if (avail_in_ == 0) {
    *des_size = i;
    return GifDecodeStatus::Unfinished;
  }
  return GifDecodeStatus::Error;
}

void CFX_LZWDecoder::ClearTable() {
  code_size_cur_ = code_size_ + 1;
  code_next_ = code_end_ + 1;
  code_old_ = static_cast<uint16_t>(-1);
  memset(code_table_, 0, sizeof(tag_Table) * GIF_MAX_LZW_CODE);
  memset(stack_, 0, GIF_MAX_LZW_CODE);
  for (uint16_t i = 0; i < code_clear_; i++)
    code_table_[i].suffix = static_cast<uint8_t>(i);
}

void CFX_LZWDecoder::AddCode(uint16_t prefix_code, uint8_t append_char) {
  if (code_next_ == GIF_MAX_LZW_CODE)
    return;
  code_table_[code_next_].prefix = prefix_code;
  code_table_[code_next_].suffix = append_char;
  if (++code_next_ < GIF_MAX_LZW_CODE) {
    if (code_next_ >> code_size_cur_)
      code_size_cur_++;
  }
}

bool CFX_LZWDecoder::DecodeString(uint16_t code) {
  stack_size_ = 0;
  while (code >= code_clear_ && code <= code_next_) {
    if (code == code_table_[code].prefix || stack_size_ == GIF_MAX_LZW_CODE - 1)
      return false;
    stack_[GIF_MAX_LZW_CODE - 1 - stack_size_++] = code_table_[code].suffix;
    code = code_table_[code].prefix;
  }
  if (code >= code_color_end_)
    return false;
  stack_[GIF_MAX_LZW_CODE - 1 - stack_size_++] = static_cast<uint8_t>(code);
  code_first_ = static_cast<uint8_t>(code);
  return true;
}
