// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_LGIF_CFX_LZWDECODER_H_
#define CORE_FXCODEC_LGIF_CFX_LZWDECODER_H_

#include <memory>
#include <vector>

#include "core/fxcodec/lgif/fx_gif.h"

class CFX_LZWDecoder {
 public:
  struct tag_Table {
    uint16_t prefix;
    uint8_t suffix;
  };

  static std::unique_ptr<CFX_LZWDecoder> MakeDecoder(char* error_ptr,
                                                     uint8_t color_exp,
                                                     uint8_t code_exp);
  explicit CFX_LZWDecoder(char* error_ptr, uint8_t color_exp, uint8_t code_exp);
  ~CFX_LZWDecoder();

  GifDecodeStatus Decode(uint8_t* src_buf,
                         uint32_t src_size,
                         uint8_t* des_buf,
                         uint32_t* des_size);

 private:
  void ClearTable();
  void AddCode(uint16_t prefix_code, uint8_t append_char);
  bool DecodeString(uint16_t code);

  uint8_t code_size_;
  uint8_t code_size_cur_;
  uint16_t code_color_end_;
  uint16_t code_clear_;
  uint16_t code_end_;
  uint16_t code_next_;
  uint8_t code_first_;
  uint8_t stack_[GIF_MAX_LZW_CODE];
  uint16_t stack_size_;
  tag_Table code_table_[GIF_MAX_LZW_CODE];
  uint16_t code_old_;

  uint8_t* next_in_;
  uint32_t avail_in_;

  uint8_t bits_left_;
  uint32_t code_store_;

  char* err_msg_;
};

#endif  // CORE_FXCODEC_LGIF_CFX_LZWDECODER_H_
