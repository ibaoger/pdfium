// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_devicecs.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CPDF_DeviceCSTest, GetRGBFromGray) {
  pdfium::Optional<std::tuple<float, float, float>> rgb;
  CPDF_DeviceCS device_gray(PDFCS_DEVICEGRAY);

  // Test normal values. For gray, only first value from buf should be used.
  float buf[3] = {0.43f, 0.11f, 0.34f};
  rgb = device_gray.GetRGB(buf);
  ASSERT_TRUE(rgb);
  EXPECT_FLOAT_EQ(0.43f, std::get<0>(*rgb));
  EXPECT_FLOAT_EQ(0.43f, std::get<1>(*rgb));
  EXPECT_FLOAT_EQ(0.43f, std::get<2>(*rgb));
  buf[0] = 0.872f;
  rgb = device_gray.GetRGB(buf);
  ASSERT_TRUE(rgb);
  EXPECT_FLOAT_EQ(0.872f, std::get<0>(*rgb));
  EXPECT_FLOAT_EQ(0.872f, std::get<1>(*rgb));
  EXPECT_FLOAT_EQ(0.872f, std::get<2>(*rgb));

  // Test boundary values
  buf[0] = {0.0f};
  rgb = device_gray.GetRGB(buf);
  ASSERT_TRUE(rgb);
  EXPECT_FLOAT_EQ(0.0f, std::get<0>(*rgb));
  EXPECT_FLOAT_EQ(0.0f, std::get<1>(*rgb));
  EXPECT_FLOAT_EQ(0.0f, std::get<2>(*rgb));
  buf[0] = 1.0f;
  rgb = device_gray.GetRGB(buf);
  ASSERT_TRUE(rgb);
  EXPECT_FLOAT_EQ(1.0f, std::get<0>(*rgb));
  EXPECT_FLOAT_EQ(1.0f, std::get<1>(*rgb));
  EXPECT_FLOAT_EQ(1.0f, std::get<2>(*rgb));

  // Test out of range values
  buf[0] = -0.01f;
  rgb = device_gray.GetRGB(buf);
  ASSERT_TRUE(rgb);
  EXPECT_FLOAT_EQ(0.0f, std::get<0>(*rgb));
  EXPECT_FLOAT_EQ(0.0f, std::get<1>(*rgb));
  EXPECT_FLOAT_EQ(0.0f, std::get<2>(*rgb));
  buf[0] = 12.5f;
  rgb = device_gray.GetRGB(buf);
  ASSERT_TRUE(rgb);
  EXPECT_FLOAT_EQ(1.0f, std::get<0>(*rgb));
  EXPECT_FLOAT_EQ(1.0f, std::get<1>(*rgb));
  EXPECT_FLOAT_EQ(1.0f, std::get<2>(*rgb));
}

TEST(CPDF_DeviceCSTest, GetRGBFromRGB) {
  pdfium::Optional<std::tuple<float, float, float>> rgb;
  CPDF_DeviceCS device_rgb(PDFCS_DEVICERGB);

  // Test normal values
  float buf[3] = {0.13f, 1.0f, 0.652f};
  rgb = device_rgb.GetRGB(buf);
  ASSERT_TRUE(rgb);
  EXPECT_FLOAT_EQ(0.13f, std::get<0>(*rgb));
  EXPECT_FLOAT_EQ(1.0f, std::get<1>(*rgb));
  EXPECT_FLOAT_EQ(0.652f, std::get<2>(*rgb));
  buf[0] = 0.0f;
  buf[1] = 0.52f;
  buf[2] = 0.78f;
  rgb = device_rgb.GetRGB(buf);
  ASSERT_TRUE(rgb);
  EXPECT_FLOAT_EQ(0.0f, std::get<0>(*rgb));
  EXPECT_FLOAT_EQ(0.52f, std::get<1>(*rgb));
  EXPECT_FLOAT_EQ(0.78f, std::get<2>(*rgb));

  // Test out of range values
  buf[0] = -10.5f;
  buf[1] = 100.0f;
  rgb = device_rgb.GetRGB(buf);
  ASSERT_TRUE(rgb);
  EXPECT_FLOAT_EQ(0.0f, std::get<0>(*rgb));
  EXPECT_FLOAT_EQ(1.0f, std::get<1>(*rgb));
  EXPECT_FLOAT_EQ(0.78f, std::get<2>(*rgb));
}

TEST(CPDF_DeviceCSTest, GetRGBFromCMYK) {
  pdfium::Optional<std::tuple<float, float, float>> rgb;
  CPDF_DeviceCS device_cmyk(PDFCS_DEVICECMYK);

  // Test normal values
  float buf[4] = {0.6f, 0.5f, 0.3f, 0.9f};
  rgb = device_cmyk.GetRGB(buf);
  ASSERT_TRUE(rgb);
  EXPECT_FLOAT_EQ(0.0627451f, std::get<0>(*rgb));
  EXPECT_FLOAT_EQ(0.0627451f, std::get<1>(*rgb));
  EXPECT_FLOAT_EQ(0.10588236f, std::get<2>(*rgb));
  buf[0] = 0.15f;
  buf[2] = 0.0f;
  rgb = device_cmyk.GetRGB(buf);
  ASSERT_TRUE(rgb);
  EXPECT_FLOAT_EQ(0.2f, std::get<0>(*rgb));
  EXPECT_FLOAT_EQ(0.0862745f, std::get<1>(*rgb));
  EXPECT_FLOAT_EQ(0.16470589f, std::get<2>(*rgb));
  buf[2] = 1.0f;
  buf[3] = 0.0f;
  rgb = device_cmyk.GetRGB(buf);
  ASSERT_TRUE(rgb);
  EXPECT_FLOAT_EQ(0.85098046f, std::get<0>(*rgb));
  EXPECT_FLOAT_EQ(0.552941f, std::get<1>(*rgb));
  EXPECT_FLOAT_EQ(0.15686275f, std::get<2>(*rgb));

  // Test out of range values
  buf[2] = 1.5f;
  buf[3] = -0.6f;
  rgb = device_cmyk.GetRGB(buf);
  ASSERT_TRUE(rgb);
  EXPECT_FLOAT_EQ(0.85098046f, std::get<0>(*rgb));
  EXPECT_FLOAT_EQ(0.552941f, std::get<1>(*rgb));
  EXPECT_FLOAT_EQ(0.15686275f, std::get<2>(*rgb));
}
