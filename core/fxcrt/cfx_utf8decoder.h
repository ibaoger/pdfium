// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_UTF8DECODER_H_
#define CORE_FXCRT_CFX_UTF8DECODER_H_

#include "core/fxcrt/cfx_widetextbuf.h"

class CFX_UTF8Decoder {
 public:
  CFX_UTF8Decoder() { m_PendingBytes = 0; }

  void Clear();
  void Input(uint8_t byte);
  void AppendCodePoint(uint32_t ch);
  void ClearStatus() { m_PendingBytes = 0; }
  CFX_WideStringC GetResult() const { return m_Buffer.AsStringC(); }

 private:
  int m_PendingBytes;
  uint32_t m_PendingChar;
  CFX_WideTextBuf m_Buffer;
};

#endif  // CORE_FXCRT_CFX_UTF8DECODER_H_
