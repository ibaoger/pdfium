// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_COMMON_BC_COMMONBITMATRIX_H_
#define FXBARCODE_COMMON_BC_COMMONBITMATRIX_H_

#include "core/fxcrt/fx_system.h"

class CBC_CommonBitArray;

class CBC_CommonBitMatrix {
 public:
  CBC_CommonBitMatrix();
  ~CBC_CommonBitMatrix();

  void Init(int32_t dimension);
  void Init(int32_t width, int32_t height);

  bool Get(int32_t x, int32_t y);
  void Set(int32_t x, int32_t y);
  void Flip(int32_t x, int32_t y);
  void Clear();
  bool SetRegion(int32_t left, int32_t top, int32_t width, int32_t height);
  CBC_CommonBitArray* GetRow(int32_t y, CBC_CommonBitArray* row);
  void SetRow(int32_t y, CBC_CommonBitArray* row);
  CBC_CommonBitArray* GetCol(int32_t y, CBC_CommonBitArray* row);
  void SetCol(int32_t y, CBC_CommonBitArray* col);
  int32_t GetWidth();
  int32_t GetHeight();
  int32_t GetRowSize();
  int32_t* GetBits();

 private:
  int32_t m_width;
  int32_t m_height;
  int32_t m_rowSize;
  int32_t* m_bits;
};

#endif  // FXBARCODE_COMMON_BC_COMMONBITMATRIX_H_
