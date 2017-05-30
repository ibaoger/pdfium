// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_annot.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFAnnotEmbeddertest : public EmbedderTest {};

TEST_F(FPDFAnnotEmbeddertest, ExtractHighlightLongContent) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // Check that there is a total of 1 annotation on its first page.
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));

  // Check that the annotation is of type "highlight".
  FPDF_ANNOTATION annot;
  ASSERT_TRUE(FPDFPage_GetAnnot(page, 0, &annot));
  EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnotation_GetSubtype(annot));

  // Check that the annotation color is yellow.
  unsigned int R, G, B, A;
  EXPECT_TRUE(FPDFAnnotation_GetColor(annot, &R, &G, &B, &A));
  EXPECT_EQ(255u, R);
  EXPECT_EQ(255u, G);
  EXPECT_EQ(0u, B);
  EXPECT_EQ(255u, A);

  // Check that the author is correct.
  const int author_size = 16;
  char* author_buffer = new char[author_size];
  unsigned long author_len =
      FPDFAnnotation_GetText(annot, author_buffer, author_size, Author);
  EXPECT_EQ(14u, author_len);
  const char author[] = "Jae Hyun Park";
  EXPECT_STREQ(author, author_buffer);

  // Check that the content is correct.
  const int contents_size = 2048;
  char* contents_buffer = new char[contents_size];
  unsigned long contents_len =
      FPDFAnnotation_GetText(annot, contents_buffer, contents_size);
  EXPECT_EQ(1345u, contents_len);
  const char contents[] =
      "This is a note for that highlight annotation. Very long highlight "
      "annotation. Long long long Long long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long long. END";
  EXPECT_STREQ(contents, contents_buffer);

  // Check that the quadpoints are correct.
  FS_QUADPOINTSF quadpoints;
  ASSERT_TRUE(FPDFAnnotation_GetQuadPoints(annot, &quadpoints));
  EXPECT_EQ(115.802643f, quadpoints.x1);
  EXPECT_EQ(718.913940f, quadpoints.y1);
  EXPECT_EQ(157.211182f, quadpoints.x4);
  EXPECT_EQ(706.264465f, quadpoints.y4);
}

TEST_F(FPDFAnnotEmbeddertest, ExtractInkMultiple) {
  // Open a file with three annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // Check that there is a total of 3 annotation on its first page.
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(page));

  // Check that the third annotation of type "ink".
  FPDF_ANNOTATION annot;
  ASSERT_TRUE(FPDFPage_GetAnnot(page, 2, &annot));
  EXPECT_EQ(FPDF_ANNOT_INK, FPDFAnnotation_GetSubtype(annot));

  // Check that the annotation color is blue with opacity.
  unsigned int R, G, B, A;
  EXPECT_TRUE(FPDFAnnotation_GetColor(annot, &R, &G, &B, &A));
  EXPECT_EQ(0u, R);
  EXPECT_EQ(0u, G);
  EXPECT_EQ(255u, B);
  EXPECT_EQ(76u, A);

  // Check that there is no content.
  const int contents_size = 4;
  char* contents_buffer = new char[contents_size];
  unsigned long contents_len =
      FPDFAnnotation_GetText(annot, contents_buffer, contents_size);
  EXPECT_EQ(1u, contents_len);
  EXPECT_STREQ("", contents_buffer);

  // Check that the rectange coordinates are correct.
  // Note that upon rendering, the rectangle coordinates will be inflated.
  FS_RECTF rect;
  ASSERT_TRUE(FPDFAnnotation_GetRect(annot, &rect));
  EXPECT_EQ(351.820404f, rect.left);
  EXPECT_EQ(583.830688f, rect.bottom);
  EXPECT_EQ(475.336090f, rect.right);
  EXPECT_EQ(681.535034f, rect.top);
}
