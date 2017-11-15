// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "public/cpp/fpdf_deleters.h"
#include "public/fpdf_ext.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFExtEmbeddertest : public EmbedderTest {};

TEST_F(FPDFExtEmbeddertest, PageModeUnknown) {
  EXPECT_EQ(PAGEMODE_UNKNOWN, FPDFDoc_GetPageMode(nullptr));
}

TEST_F(FPDFExtEmbeddertest, PageModeUseNone) {
  EXPECT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_EQ(PAGEMODE_USENONE, FPDFDoc_GetPageMode(document()));
}

TEST_F(FPDFExtEmbeddertest, PageModeUseOutlines) {
  EXPECT_TRUE(OpenDocument("use_outlines.pdf"));
  EXPECT_EQ(PAGEMODE_USEOUTLINES, FPDFDoc_GetPageMode(document()));
}

TEST_F(FPDFExtEmbeddertest, Bug_782596) {
  EXPECT_TRUE(OpenDocument("bug_782596.pdf"));
  std::unique_ptr<void, FPDFPageDeleter> page(FPDF_LoadPage(document(), 0));
  EXPECT_NE(nullptr, page.get());
  std::unique_ptr<void, FPDFTextPageDeleter> text_page(
      FPDFText_LoadPage(page.get()));
  EXPECT_NE(nullptr, text_page.get());
}
