// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>

#include "public/fpdf_edit.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gmock/include/gmock/gmock-matchers.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

class FPDFEditEmbeddertest : public EmbedderTest, public TestSaver {};

namespace {

const char kExpectedPDF[] =
    "%PDF-1.7\r\n"
    "%\xA1\xB3\xC5\xD7\r\n"
    "1 0 obj\r\n"
    "<</Pages 2 0 R /Type/Catalog>>\r\n"
    "endobj\r\n"
    "2 0 obj\r\n"
    "<</Count 1/Kids\\[ 4 0 R \\]/Type/Pages>>\r\n"
    "endobj\r\n"
    "3 0 obj\r\n"
    "<</CreationDate\\(D:.*\\)/Creator\\(PDFium\\)>>\r\n"
    "endobj\r\n"
    "4 0 obj\r\n"
    "<</Contents 5 0 R /MediaBox\\[ 0 0 640 480\\]"
    "/Parent 2 0 R /Resources<<>>/Rotate 0/Type/Page"
    ">>\r\n"
    "endobj\r\n"
    "5 0 obj\r\n"
    "<</Filter/FlateDecode/Length 8>>stream\r\n"
    // Character '_' is matching '\0' (see comment below).
    "x\x9C\x3____\x1\r\n"
    "endstream\r\n"
    "endobj\r\n"
    "xref\r\n"
    "0 6\r\n"
    "0000000000 65535 f\r\n"
    "0000000017 00000 n\r\n"
    "0000000066 00000 n\r\n"
    "0000000122 00000 n\r\n"
    "0000000192 00000 n\r\n"
    "0000000301 00000 n\r\n"
    "trailer\r\n"
    "<<\r\n"
    "/Root 1 0 R\r\n"
    "/Info 3 0 R\r\n"
    "/Size 6/ID\\[<.*><.*>\\]>>\r\n"
    "startxref\r\n"
    "379\r\n"
    "%%EOF\r\n";

int GetBlockFromString(void* param,
                       unsigned long pos,
                       unsigned char* buf,
                       unsigned long size) {
  std::string* new_file = static_cast<std::string*>(param);
  if (!new_file || pos + size < pos)
    return 0;

  unsigned long file_size = new_file->size();
  if (pos + size > file_size)
    return 0;

  memcpy(buf, new_file->data() + pos, size);
  return 1;
}

}  // namespace

TEST_F(FPDFEditEmbeddertest, EmptyCreation) {
  EXPECT_TRUE(CreateEmptyDocument());
  FPDF_PAGE page = FPDFPage_New(document(), 0, 640.0, 480.0);
  EXPECT_NE(nullptr, page);
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // The MatchesRegexp doesn't support embedded NUL ('\0') characters. They are
  // replaced by '_' for the purpose of the test.
  std::string result = GetString();
  std::replace(result.begin(), result.end(), '\0', '_');
  EXPECT_THAT(result, testing::MatchesRegex(
                          std::string(kExpectedPDF, sizeof(kExpectedPDF))));
  FPDF_ClosePage(page);
}

// Regression test for https://crbug.com/667012
TEST_F(FPDFEditEmbeddertest, RasterizePDF) {
  const char kAllBlackMd5sum[] = "5708fc5c4a8bd0abde99c8e8f0390615";

  // Get the bitmap for the original document/
  FPDF_BITMAP orig_bitmap;
  {
    EXPECT_TRUE(OpenDocument("black.pdf"));
    FPDF_PAGE orig_page = LoadPage(0);
    EXPECT_NE(nullptr, orig_page);
    orig_bitmap = RenderPage(orig_page);
    CompareBitmap(orig_bitmap, 612, 792, kAllBlackMd5sum);
    UnloadPage(orig_page);
  }

  // Create a new document from |orig_bitmap| and save it.
  {
    FPDF_DOCUMENT temp_doc = FPDF_CreateNewDocument();
    FPDF_PAGE temp_page = FPDFPage_New(temp_doc, 0, 612, 792);

    // Add the bitmap to an image object and add the image object to the output
    // page.
    FPDF_PAGEOBJECT temp_img = FPDFPageObj_NewImgeObj(temp_doc);
    EXPECT_TRUE(FPDFImageObj_SetBitmap(&temp_page, 1, temp_img, orig_bitmap));
    EXPECT_TRUE(FPDFImageObj_SetMatrix(temp_img, 612, 0, 0, 792, 0, 0));
    FPDFPage_InsertObject(temp_page, temp_img);
    EXPECT_TRUE(FPDFPage_GenerateContent(temp_page));
    EXPECT_TRUE(FPDF_SaveAsCopy(temp_doc, this, 0));
    FPDF_ClosePage(temp_page);
    FPDF_CloseDocument(temp_doc);
  }
  FPDFBitmap_Destroy(orig_bitmap);

  // Get the generated content. Make sure it is at least as big as the original
  // PDF.
  std::string new_file = GetString();
  EXPECT_GT(new_file.size(), 923U);

  // Read |new_file| in, and verify its rendered bitmap.
  {
    FPDF_FILEACCESS file_access;
    memset(&file_access, 0, sizeof(file_access));
    file_access.m_FileLen = new_file.size();
    file_access.m_GetBlock = GetBlockFromString;
    file_access.m_Param = &new_file;

    FPDF_DOCUMENT new_doc = FPDF_LoadCustomDocument(&file_access, nullptr);
    EXPECT_EQ(1, FPDF_GetPageCount(document_));
    FPDF_PAGE new_page = FPDF_LoadPage(new_doc, 0);
    EXPECT_NE(nullptr, new_page);
    FPDF_BITMAP new_bitmap = RenderPage(new_page);
    CompareBitmap(new_bitmap, 612, 792, kAllBlackMd5sum);
    FPDF_ClosePage(new_page);
    FPDF_CloseDocument(new_doc);
    FPDFBitmap_Destroy(new_bitmap);
  }
}

TEST_F(FPDFEditEmbeddertest, AddPaths) {
  // Start with a blank page
  FPDF_DOCUMENT doc = FPDF_CreateNewDocument();
  FPDF_PAGE page = FPDFPage_New(doc, 0, 612, 792);

  // We will first add a red rectangle
  FPDF_PAGEOBJECT red_rect = FPDFPageObj_CreateNewRect(10, 10, 20, 20);
  ASSERT_NE(nullptr, red_rect);
  // Expect false when trying to set colors out of range
  EXPECT_FALSE(FPDFPath_SetStrokeColor(red_rect, 100, 100, 100, 300));
  EXPECT_FALSE(FPDFPath_SetFillColor(red_rect, 200, 256, 200, 0));

  // Fill rectangle with red and insert to the page
  EXPECT_TRUE(FPDFPath_SetFillColor(red_rect, 255, 0, 0, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(page, red_rect);
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  FPDF_BITMAP page_bitmap = RenderPage(page);
  CompareBitmap(page_bitmap, 612, 792, "66d02eaa6181e2c069ce2ea99beda497");
  FPDFBitmap_Destroy(page_bitmap);

  // Now add to that a green rectangle with some medium alpha
  FPDF_PAGEOBJECT green_rect = FPDFPageObj_CreateNewRect(100, 100, 40, 40);
  EXPECT_TRUE(FPDFPath_SetFillColor(green_rect, 0, 255, 0, 128));
  EXPECT_TRUE(FPDFPath_SetDrawMode(green_rect, FPDF_FILLMODE_WINDING, 0));
  FPDFPage_InsertObject(page, green_rect);
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  page_bitmap = RenderPage(page);
  CompareBitmap(page_bitmap, 612, 792, "7b0b87604594e773add528fae567a558");
  FPDFBitmap_Destroy(page_bitmap);

  // Add a black triangle.
  FPDF_PAGEOBJECT black_path = FPDFPageObj_CreateNewPath(400, 100);
  EXPECT_TRUE(FPDFPath_SetFillColor(black_path, 0, 0, 0, 200));
  EXPECT_TRUE(FPDFPath_SetDrawMode(black_path, FPDF_FILLMODE_ALTERNATE, 0));
  EXPECT_TRUE(FPDFPath_LineTo(black_path, 400, 200));
  EXPECT_TRUE(FPDFPath_LineTo(black_path, 300, 100));
  EXPECT_TRUE(FPDFPath_Close(black_path));
  FPDFPage_InsertObject(page, black_path);
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  page_bitmap = RenderPage(page);
  CompareBitmap(page_bitmap, 612, 792, "eadc8020a14dfcf091da2688733d8806");
  FPDFBitmap_Destroy(page_bitmap);

  // Now add a more complex blue path.
  FPDF_PAGEOBJECT blue_path = FPDFPageObj_CreateNewPath(200, 200);
  EXPECT_TRUE(FPDFPath_SetFillColor(blue_path, 0, 0, 255, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(blue_path, FPDF_FILLMODE_WINDING, 0));
  EXPECT_TRUE(FPDFPath_LineTo(blue_path, 230, 230));
  EXPECT_TRUE(FPDFPath_BezierTo(blue_path, 250, 250, 280, 280, 300, 300));
  EXPECT_TRUE(FPDFPath_LineTo(blue_path, 325, 325));
  EXPECT_TRUE(FPDFPath_LineTo(blue_path, 350, 325));
  EXPECT_TRUE(FPDFPath_BezierTo(blue_path, 375, 330, 390, 360, 400, 400));
  EXPECT_TRUE(FPDFPath_Close(blue_path));
  FPDFPage_InsertObject(page, blue_path);
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  page_bitmap = RenderPage(page);
  CompareBitmap(page_bitmap, 612, 792, "9823e1a21bd9b72b6a442ba4f12af946");
  FPDFBitmap_Destroy(page_bitmap);

  // Now save the result, closing the page and document
  EXPECT_TRUE(FPDF_SaveAsCopy(doc, this, 0));
  FPDF_ClosePage(page);
  FPDF_CloseDocument(doc);
  std::string new_file = GetString();

  // Render the saved result
  FPDF_FILEACCESS file_access;
  memset(&file_access, 0, sizeof(file_access));
  file_access.m_FileLen = new_file.size();
  file_access.m_GetBlock = GetBlockFromString;
  file_access.m_Param = &new_file;
  FPDF_DOCUMENT new_doc = FPDF_LoadCustomDocument(&file_access, nullptr);
  ASSERT_NE(nullptr, new_doc);
  EXPECT_EQ(1, FPDF_GetPageCount(new_doc));
  FPDF_PAGE new_page = FPDF_LoadPage(new_doc, 0);
  ASSERT_NE(nullptr, new_page);
  FPDF_BITMAP new_bitmap = RenderPage(new_page);
  CompareBitmap(new_bitmap, 612, 792, "9823e1a21bd9b72b6a442ba4f12af946");
  FPDFBitmap_Destroy(new_bitmap);
  FPDF_ClosePage(new_page);
  FPDF_CloseDocument(new_doc);
}

TEST_F(FPDFEditEmbeddertest, PathOnTopOfText) {
  // Load document with some text
  EXPECT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);

  // Add an opaque rectangle on top of some of the text.
  FPDF_PAGEOBJECT red_rect = FPDFPageObj_CreateNewRect(20, 100, 50, 50);
  EXPECT_TRUE(FPDFPath_SetFillColor(red_rect, 255, 0, 0, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(page, red_rect);
  EXPECT_TRUE(FPDFPage_GenerateContent(page));

  // Add a transparent triangle on top of other part of the text.
  FPDF_PAGEOBJECT black_path = FPDFPageObj_CreateNewPath(20, 50);
  EXPECT_TRUE(FPDFPath_SetFillColor(black_path, 0, 0, 0, 100));
  EXPECT_TRUE(FPDFPath_SetDrawMode(black_path, FPDF_FILLMODE_ALTERNATE, 0));
  EXPECT_TRUE(FPDFPath_LineTo(black_path, 30, 80));
  EXPECT_TRUE(FPDFPath_LineTo(black_path, 40, 10));
  EXPECT_TRUE(FPDFPath_Close(black_path));
  FPDFPage_InsertObject(page, black_path);
  EXPECT_TRUE(FPDFPage_GenerateContent(page));

  // Render and check the result
  FPDF_BITMAP new_bitmap = RenderPage(page);
  CompareBitmap(new_bitmap, 200, 200, "17c942c76ff229200f2c98073bb60d85");
  UnloadPage(page);
}
