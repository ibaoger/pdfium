// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_extension.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(fxcrt, FXSYS_HexCharToInt) {
  EXPECT_EQ(10, FXSYS_HexCharToInt('a'));
  EXPECT_EQ(10, FXSYS_HexCharToInt('A'));
  EXPECT_EQ(7, FXSYS_HexCharToInt('7'));
  EXPECT_EQ(0, FXSYS_HexCharToInt('i'));
}

TEST(fxcrt, FXSYS_DecimalCharToInt) {
  EXPECT_EQ(7, FXSYS_DecimalCharToInt('7'));
  EXPECT_EQ(0, FXSYS_DecimalCharToInt('a'));
  EXPECT_EQ(7, FXSYS_DecimalCharToInt(L'7'));
  EXPECT_EQ(0, FXSYS_DecimalCharToInt(L'a'));
}

TEST(fxcrt, FXSYS_isDecimalDigit) {
  EXPECT_TRUE(FXSYS_isDecimalDigit('7'));
  EXPECT_TRUE(FXSYS_isDecimalDigit(L'7'));
  EXPECT_FALSE(FXSYS_isDecimalDigit('a'));
  EXPECT_FALSE(FXSYS_isDecimalDigit(L'a'));
}

TEST(fxcrt, FX_HashCode_Ascii) {
  EXPECT_EQ(0u, FX_HashCode_GetA("", false));
  EXPECT_EQ(65u, FX_HashCode_GetA("A", false));
  EXPECT_EQ(97u, FX_HashCode_GetA("A", true));
  EXPECT_EQ(31 * 65u + 66u, FX_HashCode_GetA("AB", false));
}

TEST(fxcrt, FX_HashCode_Wide) {
  EXPECT_EQ(0u, FX_HashCode_GetW(L"", false));
  EXPECT_EQ(65u, FX_HashCode_GetW(L"A", false));
  EXPECT_EQ(97u, FX_HashCode_GetW(L"A", true));
  EXPECT_EQ(1313 * 65u + 66u, FX_HashCode_GetW(L"AB", false));
}

TEST(fxcrt, FXSYS_IntToTwoHexChars) {
  char buf[2];
  FXSYS_IntToTwoHexChars(0x0, buf);
  EXPECT_EQ(0, strncmp(buf, "00", 2));
  FXSYS_IntToTwoHexChars(0x9, buf);
  EXPECT_EQ(0, strncmp(buf, "09", 2));
  FXSYS_IntToTwoHexChars(0xA, buf);
  EXPECT_EQ(0, strncmp(buf, "0A", 2));
  FXSYS_IntToTwoHexChars(0x8C, buf);
  EXPECT_EQ(0, strncmp(buf, "8C", 2));
  FXSYS_IntToTwoHexChars(0xBE, buf);
  EXPECT_EQ(0, strncmp(buf, "BE", 2));
  FXSYS_IntToTwoHexChars(0xD0, buf);
  EXPECT_EQ(0, strncmp(buf, "D0", 2));
  FXSYS_IntToTwoHexChars(0xFF, buf);
  EXPECT_EQ(0, strncmp(buf, "FF", 2));
}

TEST(fxcrt, FXSYS_IntToFourHexChars) {
  char buf[4];
  FXSYS_IntToFourHexChars(0x0, buf);
  EXPECT_EQ(0, strncmp(buf, "0000", 4));
  FXSYS_IntToFourHexChars(0xA23, buf);
  EXPECT_EQ(0, strncmp(buf, "0A23", 4));
  FXSYS_IntToFourHexChars(0xB701, buf);
  EXPECT_EQ(0, strncmp(buf, "B701", 4));
  FXSYS_IntToFourHexChars(0xFFFF, buf);
  EXPECT_EQ(0, strncmp(buf, "FFFF", 4));
}

TEST(fxcrt, FXSYS_ToUTF16BE) {
  char buf[8];
  // Test U+0000 to U+D7FF and U+E000 to U+FFFF
  EXPECT_EQ(4U, FXSYS_ToUTF16BE(0x0, buf));
  EXPECT_EQ(0, strncmp(buf, "0000", 4));
  EXPECT_EQ(4U, FXSYS_ToUTF16BE(0xD7FF, buf));
  EXPECT_EQ(0, strncmp(buf, "D7FF", 4));
  EXPECT_EQ(4U, FXSYS_ToUTF16BE(0xE000, buf));
  EXPECT_EQ(0, strncmp(buf, "E000", 4));
  EXPECT_EQ(4U, FXSYS_ToUTF16BE(0xFFFF, buf));
  EXPECT_EQ(0, strncmp(buf, "FFFF", 4));
  // Test U+10000 to U+10FFFF
  EXPECT_EQ(8U, FXSYS_ToUTF16BE(0x10000, buf));
  EXPECT_EQ(0, strncmp(buf, "D800DC00", 8));
  EXPECT_EQ(8U, FXSYS_ToUTF16BE(0x10FFFF, buf));
  EXPECT_EQ(0, strncmp(buf, "DBFFDFFF", 8));
  EXPECT_EQ(8U, FXSYS_ToUTF16BE(0x2003E, buf));
  EXPECT_EQ(0, strncmp(buf, "D840DC3E", 8));
}
