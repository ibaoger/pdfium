// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxbarcode/oned/BC_OnedEAN13Writer.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

struct TestCase {
  const char* input;
  int32_t checksum;
  int32_t patterns[7];
  size_t num_patterns;
};

TEST(OnedEAN13WriterTest, Encode) {
  CBC_OnedEAN13Writer writer;
  int32_t width;
  int32_t height;
  uint8_t* encoded;
  const char* expected;

  // EAN-13 barcodes encode 13-digit numbers into 95 modules in a unidimensional
  // disposition.
  encoded = writer.Encode("", BCFORMAT_EAN_13, width, height);
  EXPECT_EQ(nullptr, encoded);
  FX_Free(encoded);

  encoded = writer.Encode("123", BCFORMAT_EAN_13, width, height);
  EXPECT_EQ(nullptr, encoded);
  FX_Free(encoded);

  encoded = writer.Encode("123456789012", BCFORMAT_EAN_13, width, height);
  EXPECT_EQ(nullptr, encoded);
  FX_Free(encoded);

  encoded = writer.Encode("12345678901234", BCFORMAT_EAN_13, width, height);
  EXPECT_EQ(nullptr, encoded);
  FX_Free(encoded);

  encoded = writer.Encode("1234567890128", BCFORMAT_EAN_13, width, height);
  EXPECT_NE(nullptr, encoded);
  EXPECT_EQ(1, height);
  EXPECT_EQ(95, width);

  expected =
      "# #"  // Start
      // 1 implicit by LLGLGG in next 6 digits
      "  #  ##"  // 2 L
      " #### #"  // 3 L
      "  ### #"  // 4 G
      " ##   #"  // 5 L
      "    # #"  // 6 G
      "  #   #"  // 7 G
      " # # "    // Middle
      "#  #   "  // 8 R
      "### #  "  // 9 R
      "###  # "  // 0 R
      "##  ## "  // 1 R
      "## ##  "  // 2 R
      "#  #   "  // 8 R
      "# #";     // End
  for (int i = 0; i < 95; i++) {
    EXPECT_EQ(expected[i] != ' ', !!encoded[i]) << i;
  }

  encoded = writer.Encode("7776665554440", BCFORMAT_EAN_13, width, height);
  EXPECT_NE(nullptr, encoded);
  EXPECT_EQ(1, height);
  EXPECT_EQ(95, width);

  expected =
      "# #"  // Start
      // 7 implicit by LGLGLG in next 6 digits
      " ### ##"  // 7 L
      "  #   #"  // 7 G
      " # ####"  // 6 L
      "    # #"  // 6 G
      " # ####"  // 6 L
      " ###  #"  // 5 G
      " # # "    // Middle
      "#  ### "  // 5 R
      "#  ### "  // 5 R
      "# ###  "  // 4 R
      "# ###  "  // 4 R
      "# ###  "  // 4 R
      "###  # "  // 0 R
      "# #";     // End
  for (int i = 0; i < 95; i++) {
    EXPECT_EQ(expected[i] != ' ', !!encoded[i]) << i;
  }

  FX_Free(encoded);
}

TEST(OnedEAN13WriterTest, Checksum) {
  CBC_OnedEAN13Writer writer;
  EXPECT_EQ(0, writer.CalcChecksum(""));
  EXPECT_EQ(6, writer.CalcChecksum("123"));
  EXPECT_EQ(8, writer.CalcChecksum("123456789012"));
  EXPECT_EQ(0, writer.CalcChecksum("777666555444"));
}

// TEST(OnedEAN13WriterTest, Encode128C) {
//   char buf[100];
//   TestCase kTestCases[] = {
//       {"", 105, {105}, 1},
//       {"a", 202, {105, 97}, 2},
//       {"1", 106, {105, 1}, 2},
//       {"a1", 204, {105, 97, 1}, 3},
//       {"ab", 398, {105, 97, 98}, 3},
//       {"12", 117, {105, 12}, 2},
//       {"abc", 695, {105, 97, 98, 99}, 4},
//       {"123", 123, {105, 12, 3}, 3},
//       {"abc123", 758, {105, 97, 98, 99, 12, 3}, 6},
//       {"ABC123", 566, {105, 65, 66, 67, 12, 3}, 6},
//       {"321ABC", 933, {105, 32, 1, 65, 66, 67}, 6},
//       {"XYZ", 641, {105, 88, 89, 90}, 4},
//   };
//   for (size_t i = 0; i < FX_ArraySize(kTestCases); ++i) {
//     FXSYS_snprintf(buf, sizeof(buf) - 1, "Test case %zu", i);
//     SCOPED_TRACE(buf);
//     const TestCase& test_case = kTestCases[i];
//     std::vector<int32_t> patterns;
//     int32_t checksum =
//         CBC_OnedCode128Writer::Encode128C(test_case.input, &patterns);
//     EXPECT_EQ(test_case.checksum, checksum);
//     ASSERT_EQ(test_case.num_patterns, patterns.size());
//     for (size_t j = 0; j < patterns.size(); ++j) {
//       FXSYS_snprintf(buf, sizeof(buf) - 1, "Comparison %zu", j);
//       SCOPED_TRACE(buf);
//       EXPECT_EQ(test_case.patterns[j], patterns[j]);
//     }
//   }
// }

// TEST(OnedEAN13WriterTest, CheckContentValidity) {
//   {
//     CBC_OnedCode128Writer writer(BC_CODE128_B);
//     EXPECT_TRUE(writer.CheckContentValidity(L""));
//     EXPECT_TRUE(writer.CheckContentValidity(L"foo"));
//     EXPECT_TRUE(writer.CheckContentValidity(L"xyz"));
//     EXPECT_FALSE(writer.CheckContentValidity(L"\""));
//     EXPECT_FALSE(writer.CheckContentValidity(L"f\x10oo"));
//     EXPECT_FALSE(writer.CheckContentValidity(L"bar\x7F"));
//     EXPECT_FALSE(writer.CheckContentValidity(L"qux\x88"));
//   }
//   {
//     CBC_OnedCode128Writer writer(BC_CODE128_C);
//     EXPECT_TRUE(writer.CheckContentValidity(L""));
//     EXPECT_TRUE(writer.CheckContentValidity(L"foo"));
//     EXPECT_TRUE(writer.CheckContentValidity(L"xyz"));
//     EXPECT_FALSE(writer.CheckContentValidity(L"\""));
//     EXPECT_FALSE(writer.CheckContentValidity(L"f\x10oo"));
//     EXPECT_FALSE(writer.CheckContentValidity(L"bar\x7F"));
//     EXPECT_FALSE(writer.CheckContentValidity(L"qux\x88"));
//   }
// }

// TEST(OnedEAN13WriterTest, FilterContents) {
//   {
//     CBC_OnedCode128Writer writer(BC_CODE128_B);
//     EXPECT_STREQ(L"", writer.FilterContents(L"").c_str());
//     EXPECT_STREQ(L"foo", writer.FilterContents(L"foo\x10").c_str());
//     EXPECT_STREQ(L"fool", writer.FilterContents(L"foo\x10l").c_str());
//     EXPECT_STREQ(L"foo", writer.FilterContents(L"foo\x10\x7F").c_str());
//     EXPECT_STREQ(L"foo", writer.FilterContents(L"foo\x10\x7F\x88").c_str());
//     EXPECT_STREQ(L"bar", writer.FilterContents(L"bar\x10\x7F\x88").c_str());
//   }
//   {
//     CBC_OnedCode128Writer writer(BC_CODE128_C);
//     EXPECT_STREQ(L"", writer.FilterContents(L"").c_str());
//     EXPECT_STREQ(L"f", writer.FilterContents(L"foo\x10").c_str());
//     EXPECT_STREQ(L"f", writer.FilterContents(L"foo\x10l").c_str());
//     EXPECT_STREQ(L"f", writer.FilterContents(L"foo\x10\x7F").c_str());
//     EXPECT_STREQ(L"f", writer.FilterContents(L"foo\x10\x7F\x88").c_str());
//     EXPECT_STREQ(L"ba", writer.FilterContents(L"bar\x10\x7F\x88").c_str());
//   }
// }

}  // namespace
