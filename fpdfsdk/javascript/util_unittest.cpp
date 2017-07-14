// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/javascript/util.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

// Should mirror the defines in util.cpp.
#define UTIL_INT 0
#define UTIL_DOUBLE 1
#define UTIL_STRING 2

TEST(CJS_Util, ParseDataType) {
  struct ParseDataTypeCase {
    const wchar_t* const input_string;
    const int expected;
  };

  // Commented out tests follow the spec but are not passing.
  const ParseDataTypeCase cases[] = {
      // Not conversions
      {L"", -1},
      {L"d", -1},

      // Simple cases
      {L"%d", 0},
      {L"%x", 0},
      {L"%f", 1},
      {L"%s", 2},

      // nDecSep Not implemented
      // {L"%,0d", 0},
      // {L"%,1d", 0},
      // {L"%,2d", 0},
      // {L"%,3d", 0},
      // {L"%,4d", -1},
      // {L"%,d", -1},

      // cFlags("+ 0#"") are only valid for numeric conversions.
      {L"%+d", 0},
      {L"%+x", 0},
      {L"%+f", 1},
      // {L"%+s", -1},
      {L"% d", 0},
      {L"% x", 0},
      {L"% f", 1},
      // {L"% s", -1},
      {L"%0d", 0},
      {L"%0x", 0},
      {L"%0f", 1},
      // {L"%0s", -1},
      {L"%#d", 0},
      {L"%#x", 0},
      {L"%#f", 1},
      // {L"%#s", -1},

      // nWidth should work. for all conversions, can be combined with cFlags=0
      // for numbers.
      {L"%5d", 0},
      {L"%05d", 0},
      {L"%5x", 0},
      {L"%05x", 0},
      {L"%5f", 1},
      {L"%05f", 1},
      {L"%5s", 2},
      // {L"%05s", -1},

      // nPrecision should only work for float
      // {L"%.5d", -1},
      // {L"%.5x", -1},
      {L"%.5f", 1},
      // {L"%.5s", -1},
      // {L"%.14d", -1},
      // {L"%.14x", -1},
      {L"%.14f", 1},
      // {L"%.14s", -1},
      // {L"%.f", -1},

      // nPrecision too large (> 260) causes crashes in Windows.
      // TODO(tsepez): Reenable when fix is out.
      // {L"%.261d", -1},
      // {L"%.261x", -1},
      // {L"%.261f", -1},
      // {L"%.261s", -1},

      // Unexpected characters
      {L"%ad", -1},
      {L"%bx", -1},
      // {L"%cf", -1},
      // {L"%es", -1},
      // {L"%gd", -1},
      {L"%hx", -1},
      // {L"%if", -1},
      {L"%js", -1},
      {L"%@d", -1},
      {L"%~x", -1},
      {L"%[f", -1},
      {L"%\0s", -1},
      {L"%\nd", -1},
      {L"%\rx", -1},
      // {L"%%f", -1},
      // {L"%  s", -1},

      // Combine multiple valid components
      {L"%+6d", 0},
      {L"% 7x", 0},
      {L"%#9.3f", 1},
      {L"%10s", 2},
  };

  for (size_t i = 0; i < FX_ArraySize(cases); i++) {
    std::wstring input(cases[i].input_string);
    EXPECT_EQ(cases[i].expected, util::ParseDataType(&input)) << input;
  }
}
