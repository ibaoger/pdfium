// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_edit.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

class FPDFEditPathEmbedderTest : public EmbedderTest {};

TEST_F(FPDFEditPathEmbedderTest, VerifyCorrectColoursReturned) {
  CreateEmptyDocument();
  FPDF_PAGE page = FPDFPage_New(document(), 0, 612, 792);

  FPDF_PAGEOBJECT path = FPDFPageObj_CreateNewPath(400, 100);
  EXPECT_TRUE(FPDFPath_SetFillColor(path, 200, 201, 202, 128));
  EXPECT_TRUE(FPDFPath_SetStrokeColor(path, 100, 101, 102, 128));
  EXPECT_TRUE(FPDFPath_SetDrawMode(path, FPDF_FILLMODE_ALTERNATE, 0));
  EXPECT_TRUE(FPDFPath_LineTo(path, 400, 200));
  EXPECT_TRUE(FPDFPath_LineTo(path, 300, 100));
  EXPECT_TRUE(FPDFPath_Close(path));

  FPDFPage_InsertObject(page, path);

  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  page = nullptr;
  path = nullptr;

  FPDF_DOCUMENT doc = OpenSaved();
  page = FPDF_LoadPage(doc, 0);
  ASSERT(page);

  path = FPDFPage_GetObject(page, 0);
  ASSERT(path);

  EXPECT_EQ(FPDF_PAGEOBJ_PATH, FPDFPageObj_GetType(path));

  unsigned int r;
  unsigned int g;
  unsigned int b;
  unsigned int a;
  FPDFPath_GetFillColor(path, &r, &g, &b, &a);
  EXPECT_EQ(200u, r);
  EXPECT_EQ(201u, g);
  EXPECT_EQ(202u, b);
  EXPECT_EQ(128u, a);

  FPDFPath_GetStrokeColor(path, &r, &g, &b, &a);
  EXPECT_EQ(100u, r);
  EXPECT_EQ(101u, g);
  EXPECT_EQ(102u, b);
  EXPECT_EQ(128u, a);

  FPDF_ClosePage(page);

  CloseSaved();
}
