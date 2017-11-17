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
  static constexpr char kTransformedMD5[] = "4982be08db3f6d2e6409186ebbced9eb";
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  const int initial_width = static_cast<int>(FPDF_GetPageWidth(page));
  const int initial_height = static_cast<int>(FPDF_GetPageHeight(page));
  const float scale_factor = 0.5;
  FS_MATRIX matrix = {scale_factor, 0, 0, scale_factor, 0, 0};
  EXPECT_TRUE(FPDFPage_TransFormWithClip(page, &matrix, nullptr));
  const int width = initial_width * scale_factor;
  const int height = initial_height * scale_factor;
  FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, 0);
  FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 0xFFFFFFFF);
  FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, 0);
  CompareBitmap(bitmap, width, height, kTransformedMD5);
  FPDFBitmap_Destroy(bitmap);
  UnloadPage(page);
}

TEST_F(FPDFTransformPageEmbeddertest, FPDF_TransFormPageClipOnly) {
  static constexpr char kTransformedMD5[] = "cea217f5bf752aa11f1a7b8fd351ec8c";
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  const int initial_width = static_cast<int>(FPDF_GetPageWidth(page));
  const int initial_height = static_cast<int>(FPDF_GetPageHeight(page));
  const float scale_factor = 0.8;
  FS_RECTF cliprect = {0, 0, initial_width * scale_factor,
                       initial_height * scale_factor};
  EXPECT_TRUE(FPDFPage_TransFormWithClip(page, nullptr, &cliprect));
  const int width = initial_width * scale_factor;
  const int height = initial_height * scale_factor;
  FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, 0);
  FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 0xFFFFFFFF);
  FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, 0);
  CompareBitmap(bitmap, width, height, kTransformedMD5);
  FPDFBitmap_Destroy(bitmap);
  UnloadPage(page);
}

TEST_F(FPDFTransformPageEmbeddertest, FPDF_TransFormPageWithClip) {
  static constexpr char kTransformedMD5[] = "2ddd456a72e58644d2ef46a67ecbac70";
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  const int initial_width = static_cast<int>(FPDF_GetPageWidth(page));
  const int initial_height = static_cast<int>(FPDF_GetPageHeight(page));

  const float scale_factor = 0.1;
  FS_MATRIX matrix = {scale_factor, 0, 0, scale_factor, 0, 0};
  FS_RECTF cliprect = {0, 0, initial_width * scale_factor,
                       initial_height * scale_factor};
  EXPECT_TRUE(FPDFPage_TransFormWithClip(page, &matrix, &cliprect));
  EXPECT_EQ(initial_width, static_cast<int>(FPDF_GetPageWidth(page)));
  EXPECT_EQ(initial_height, static_cast<int>(FPDF_GetPageHeight(page)));
  const int width = initial_width * scale_factor;
  const int height = initial_height * scale_factor;
  FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, 0);
  FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 0xFFFFFFFF);
  FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, 0);
  CompareBitmap(bitmap, width, height, kTransformedMD5);
  FPDFBitmap_Destroy(bitmap);

  UnloadPage(page);
}
