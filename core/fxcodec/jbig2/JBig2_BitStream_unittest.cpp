// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcodec/jbig2/JBig2_BitStream.h"

#include <memory>
#include <utility>

#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

TEST(JBig2_BitStream, ReadNBits) {
  std::unique_ptr<uint8_t, FxFreeDeleter> data(FX_Alloc(uint8_t, 1));
  data.get()[0] = 0xff;

  auto in_stream = pdfium::MakeUnique<CPDF_Stream>(std::move(data), 1, nullptr);
  auto acc = pdfium::MakeUnique<CPDF_StreamAcc>();
  acc->LoadAllData(in_stream.get());

  CJBig2_BitStream stream(acc.get());

  uint32_t val1;
  EXPECT_EQ(0, stream.readNBits(1, &val1));
  EXPECT_EQ(1U, val1);

  int32_t val2;
  EXPECT_EQ(0, stream.readNBits(1, &val2));
  EXPECT_EQ(1, val2);
}

TEST(JBig2_BitStream, ReadNBitsOutOfBounds) {
  std::unique_ptr<uint8_t, FxFreeDeleter> data(FX_Alloc(uint8_t, 1));

  auto in_stream = pdfium::MakeUnique<CPDF_Stream>(std::move(data), 0, nullptr);
  auto acc = pdfium::MakeUnique<CPDF_StreamAcc>();
  acc->LoadAllData(in_stream.get());

  CJBig2_BitStream stream(acc.get());

  uint32_t val1;
  EXPECT_EQ(-1, stream.readNBits(1, &val1));

  int32_t val2;
  EXPECT_EQ(-1, stream.readNBits(1, &val2));
}
