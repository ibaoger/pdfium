// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>
#include <string>

#include "core/fxcrt/fx_coordinates.h"
#include "fpdfsdk/fpdfview_c_api_test.h"
#include "public/fpdf_transformpage.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gmock/include/gmock/gmock-matchers.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/path_service.h"

class FPDFTransformPageEmbeddertest : public EmbedderTest {};

// TODO(xlou): Add more unit tests to increase function coverage.

TEST_F(FPDFTransformPageEmbeddertest, FPDF_TransFormPageWithClip) {
  const char kRotatedMD5[] = "2baa4c0e1758deba1b9c908e1fbd04ed";
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  const int initial_width = static_cast<int>(FPDF_GetPageWidth(page));
  const int initial_height = static_cast<int>(FPDF_GetPageHeight(page));

  const double scale_factor = 0.5;
  const double offset_x = 1.0;
  const double offset_y = 1.0;
  FS_MATRIX matrix = {static_cast<float>(scale_factor),
                      0,
                      0,
                      static_cast<float>(scale_factor),
                      static_cast<float>(offset_x),
                      static_cast<float>(offset_y)};
  FS_RECTF cliprect = {
      static_cast<float>(offset_x), static_cast<float>(offset_y),
      static_cast<float>(offset_x), static_cast<float>(offset_y)};
  bool result = FPDFPage_TransFormWithClip(page, &matrix, &cliprect);
  EXPECT_EQ(TRANSFORMATION_SUCCESS, result);
  int width = static_cast<int>(FPDF_GetPageWidth(page));
  int height = static_cast<int>(FPDF_GetPageHeight(page));
  EXPECT_EQ(initial_width, width);
  EXPECT_EQ(initial_height, height);

  FPDF_BITMAP bitmap = RenderPage(page);
  CompareBitmap(bitmap, initial_width, initial_height, kRotatedMD5);
  FPDFBitmap_Destroy(bitmap);

  UnloadPage(page);
}
