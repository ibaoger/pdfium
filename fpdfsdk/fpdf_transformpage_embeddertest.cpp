// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_transformpage.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"

class FPDFTransformPageEmbeddertest : public EmbedderTest {};

// TODO(xlou): Add more unit tests to increase function coverage.

TEST_F(FPDFTransformPageEmbeddertest, FPDF_TransFromPageWithNullptr) {
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  EXPECT_FALSE(FPDFPage_TransFormWithClip(page, nullptr, nullptr));
  UnloadPage(page);
}

TEST_F(FPDFTransformPageEmbeddertest, FPDF_TransFormPageNoClip) {
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  FS_MATRIX matrix = {0.1, 0, 0, 0.1, 0, 0};
  EXPECT_TRUE(FPDFPage_TransFormWithClip(page, &matrix, nullptr));
  UnloadPage(page);
}

TEST_F(FPDFTransformPageEmbeddertest, FPDF_TransFormPageClipOnly) {
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  FS_RECTF cliprect = {1.0, 1.0, 1.0, 1.0};
  EXPECT_TRUE(FPDFPage_TransFormWithClip(page, nullptr, &cliprect));
  UnloadPage(page);
}

TEST_F(FPDFTransformPageEmbeddertest, FPDF_TransFormPageWithClip) {
  static constexpr char kTransformedMD5[] = "0a90de37f52127619c3dfb642b5fa2fe";
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  const int initial_width = static_cast<int>(FPDF_GetPageWidth(page));
  const int initial_height = static_cast<int>(FPDF_GetPageHeight(page));

  const float scale_factor = 0.5;
  const float offset_x = 1.0;
  const float offset_y = 1.0;
  FS_MATRIX matrix = {scale_factor, 0, 0, scale_factor, offset_x, offset_y};
  FS_RECTF cliprect = {offset_x, offset_y, offset_x, offset_y};
  EXPECT_TRUE(FPDFPage_TransFormWithClip(page, &matrix, &cliprect));
  EXPECT_EQ(initial_width, static_cast<int>(FPDF_GetPageWidth(page)));
  EXPECT_EQ(initial_height, static_cast<int>(FPDF_GetPageHeight(page)));

  FPDF_BITMAP bitmap = RenderPage(page);
  CompareBitmap(bitmap, initial_width, initial_height, kTransformedMD5);
  FPDFBitmap_Destroy(bitmap);

  UnloadPage(page);
}
