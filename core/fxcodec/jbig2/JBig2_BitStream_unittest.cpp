// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcodec/jbig2/JBig2_BitStream.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(JBig2BitStreamTest, ReadNBits) {
  const uint8_t kData[] = {0xb1};  // 10110001
  CJBig2_BitStream stream(kData, 0);

  uint32_t val1;
  EXPECT_EQ(0, stream.readNBits(1, &val1));
  EXPECT_EQ(1U, val1);

  int32_t val2;
  EXPECT_EQ(0, stream.readNBits(1, &val2));
  EXPECT_EQ(0, val2);

  EXPECT_EQ(0, stream.readNBits(2, &val2));
  EXPECT_EQ(3, val2);

  EXPECT_EQ(0, stream.readNBits(4, &val2));
  EXPECT_EQ(1, val2);
}

TEST(JBig2BitStreamTest, ReadNBitsLargerThenData) {
  const uint8_t kData[] = {0xb1};  // 10110001
  CJBig2_BitStream stream(kData, 42);

  uint32_t val1;
  EXPECT_EQ(0, stream.readNBits(10, &val1));
  EXPECT_EQ(0xb1U, val1);
}

TEST(JBig2BitStreamTest, ReadNBitsNullStream) {
  CJBig2_BitStream stream({}, 0);

  uint32_t val1;
  EXPECT_EQ(-1, stream.readNBits(1, &val1));

  int32_t val2;
  EXPECT_EQ(-1, stream.readNBits(2, &val2));
}

TEST(JBig2BitStreamTest, ReadNBitsOutOfBounds) {
  const uint8_t kData[] = {0xb1};  // 10110001
  CJBig2_BitStream stream(kData, 42);

  uint32_t val1;
  EXPECT_EQ(0, stream.readNBits(8, &val1));

  int32_t val2;
  EXPECT_EQ(-1, stream.readNBits(2, &val2));
}

TEST(JBig2BitStreamTest, ReadNBitsWhereNIs36) {
  const uint8_t kData[] = {0xb0, 0x01, 0x00, 0x00, 0x40};
  CJBig2_BitStream stream(kData, 42);

  // This will shift off the top two bits and they end up lost.
  uint32_t val1;
  EXPECT_EQ(0, stream.readNBits(34, &val1));
  EXPECT_EQ(0xc0040001U, val1);
}
