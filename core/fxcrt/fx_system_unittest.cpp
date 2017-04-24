// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>
#include <string>

#include "core/fxcrt/fx_system.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/gtest/include/gtest/gtest.h"

// Unit test covering cases where PDFium replaces well-known library
// functionality on any given platformn.

#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_

namespace {

const char kSentinel = 0x7f;

void Check32BitBase16Itoa(int32_t input, const char* expected_output) {
  const size_t kBufLen = 11;  // "-" + 8 digits + NUL + sentinel.
  char buf[kBufLen];
  buf[kBufLen - 1] = kSentinel;
  FXSYS_itoa(input, buf, 16);
  EXPECT_EQ(std::string(expected_output), buf);
  EXPECT_EQ(kSentinel, buf[kBufLen - 1]);
}

void Check32BitBase10Itoa(int32_t input, const char* expected_output) {
  const size_t kBufLen = 13;  // "-" + 10 digits + NUL + sentinel.
  char buf[kBufLen];
  buf[kBufLen - 1] = kSentinel;
  FXSYS_itoa(input, buf, 10);
  EXPECT_EQ(std::string(expected_output), buf);
  EXPECT_EQ(kSentinel, buf[kBufLen - 1]);
}

void Check32BitBase2Itoa(int32_t input, const char* expected_output) {
  const size_t kBufLen = 35;  // "-" + 32 digits + NUL + sentinel.
  char buf[kBufLen];
  buf[kBufLen - 1] = kSentinel;
  FXSYS_itoa(input, buf, 2);
  EXPECT_EQ(std::string(expected_output), buf);
  EXPECT_EQ(kSentinel, buf[kBufLen - 1]);
}

void Check64BitBase16Itoa(int64_t input, const char* expected_output) {
  const size_t kBufLen = 19;  // "-" + 16 digits + NUL + sentinel.
  char buf[kBufLen];
  buf[kBufLen - 1] = kSentinel;
  FXSYS_i64toa(input, buf, 16);
  EXPECT_EQ(std::string(expected_output), buf);
  EXPECT_EQ(kSentinel, buf[kBufLen - 1]);
}

void Check64BitBase10Itoa(int64_t input, const char* expected_output) {
  const size_t kBufLen = 22;  // "-" + 19 digits + NUL + sentinel.
  char buf[kBufLen];
  buf[kBufLen - 1] = kSentinel;
  FXSYS_i64toa(input, buf, 10);
  EXPECT_EQ(std::string(expected_output), buf);
  EXPECT_EQ(kSentinel, buf[kBufLen - 1]);
}

void Check64BitBase2Itoa(int64_t input, const char* expected_output) {
  const size_t kBufLen = 67;  // "-" + 64 digits + NUL + sentinel.
  char buf[kBufLen];
  buf[kBufLen - 1] = kSentinel;
  FXSYS_i64toa(input, buf, 2);
  EXPECT_EQ(std::string(expected_output), buf);
  EXPECT_EQ(kSentinel, buf[kBufLen - 1]);
}

}  // namespace

TEST(fxcrt, FXSYS_itoa_InvalidRadix) {
  char buf[32];

  FXSYS_itoa(42, buf, 17);  // Ours stops at 16.
  EXPECT_EQ(std::string(""), buf);

  FXSYS_itoa(42, buf, 1);
  EXPECT_EQ(std::string(""), buf);

  FXSYS_itoa(42, buf, 0);
  EXPECT_EQ(std::string(""), buf);

  FXSYS_itoa(42, buf, -1);
  EXPECT_EQ(std::string(""), buf);
}

TEST(fxcrt, FXSYS_itoa) {
  Check32BitBase16Itoa(std::numeric_limits<int32_t>::min(), "-80000000");
  Check32BitBase10Itoa(std::numeric_limits<int32_t>::min(), "-2147483648");
  Check32BitBase2Itoa(std::numeric_limits<int32_t>::min(),
                      "-10000000000000000000000000000000");

  Check32BitBase16Itoa(-1, "-1");
  Check32BitBase10Itoa(-1, "-1");
  Check32BitBase2Itoa(-1, "-1");

  Check32BitBase16Itoa(0, "0");
  Check32BitBase10Itoa(0, "0");
  Check32BitBase2Itoa(0, "0");

  Check32BitBase16Itoa(42, "2a");
  Check32BitBase10Itoa(42, "42");
  Check32BitBase2Itoa(42, "101010");

  Check32BitBase16Itoa(std::numeric_limits<int32_t>::max(), "7fffffff");
  Check32BitBase10Itoa(std::numeric_limits<int32_t>::max(), "2147483647");
  Check32BitBase2Itoa(std::numeric_limits<int32_t>::max(),
                      "1111111111111111111111111111111");
}

TEST(fxcrt, FXSYS_i64toa_InvalidRadix) {
  char buf[32];

  FXSYS_i64toa(42, buf, 17);  // Ours stops at 16.
  EXPECT_EQ(std::string(""), buf);

  FXSYS_i64toa(42, buf, 1);
  EXPECT_EQ(std::string(""), buf);

  FXSYS_i64toa(42, buf, 0);
  EXPECT_EQ(std::string(""), buf);

  FXSYS_i64toa(42, buf, -1);
  EXPECT_EQ(std::string(""), buf);
}

TEST(fxcrt, FXSYS_i64toa) {
  Check64BitBase16Itoa(std::numeric_limits<int64_t>::min(),
                       "-8000000000000000");
  Check64BitBase10Itoa(std::numeric_limits<int64_t>::min(),
                       "-9223372036854775808");
  Check64BitBase2Itoa(
      std::numeric_limits<int64_t>::min(),
      "-1000000000000000000000000000000000000000000000000000000000000000");

  Check64BitBase16Itoa(-1, "-1");
  Check64BitBase10Itoa(-1, "-1");
  Check64BitBase2Itoa(-1, "-1");

  Check64BitBase16Itoa(0, "0");
  Check64BitBase10Itoa(0, "0");
  Check64BitBase2Itoa(0, "0");

  Check64BitBase16Itoa(42, "2a");
  Check64BitBase10Itoa(42, "42");
  Check64BitBase2Itoa(42, "101010");

  Check64BitBase16Itoa(std::numeric_limits<int64_t>::max(), "7fffffffffffffff");
  Check64BitBase10Itoa(std::numeric_limits<int64_t>::max(),
                       "9223372036854775807");
  Check64BitBase2Itoa(
      std::numeric_limits<int64_t>::max(),
      "111111111111111111111111111111111111111111111111111111111111111");
}

#endif  // _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_

TEST(fxcrt, FXSYS_wcsftime) {
  struct tm good_time = {};
  good_time.tm_year = 74;  // 1900-based.
  good_time.tm_mon = 7;    // 0-based.
  good_time.tm_mday = 9;   // 1-based.
  good_time.tm_hour = 12;
  good_time.tm_min = 0;
  good_time.tm_sec = 0;

  wchar_t buf[100] = {};
  EXPECT_EQ(19u, FXSYS_wcsftime(buf, 100, L"%Y-%m-%dT%H:%M:%S", &good_time));
  EXPECT_EQ(std::wstring(L"1974-08-09T12:00:00"), buf);

  // Ensure wcsftime handles bad years, etc. without crashing.
  struct tm bad_time = {};
  bad_time.tm_year = -1;
  bad_time.tm_mon = -1;
  bad_time.tm_mday = -1;
  bad_time.tm_hour = -1;
  bad_time.tm_min = -1;
  bad_time.tm_sec = -1;

  FXSYS_wcsftime(buf, 100, L"%y-%m-%dT%H:%M:%S", &bad_time);
}
