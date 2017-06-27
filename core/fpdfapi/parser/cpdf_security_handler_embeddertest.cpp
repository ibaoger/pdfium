// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "public/fpdf_edit.h"
#include "public/fpdf_save.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class CPDFSecurityHandlerEmbeddertest : public EmbedderTest,
                                        public TestSaver {};

TEST_F(CPDFSecurityHandlerEmbeddertest, Unencrypted) {
  ASSERT_TRUE(OpenDocument("about_blank.pdf"));
  EXPECT_EQ(0xFFFFFFFF, FPDF_GetDocPermissions(document()));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, UnencryptedWithPassword) {
  ASSERT_TRUE(OpenDocument("about_blank.pdf", "foobar"));
  EXPECT_EQ(0xFFFFFFFF, FPDF_GetDocPermissions(document()));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, NoPassword) {
  EXPECT_FALSE(OpenDocument("encrypted.pdf"));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, BadPassword) {
  EXPECT_FALSE(OpenDocument("encrypted.pdf", "tiger"));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, UserPassword) {
  ASSERT_TRUE(OpenDocument("encrypted.pdf", "1234"));
  EXPECT_EQ(0xFFFFF2C0, FPDF_GetDocPermissions(document()));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, OwnerPassword) {
  ASSERT_TRUE(OpenDocument("encrypted.pdf", "5678"));
  EXPECT_EQ(0xFFFFFFFC, FPDF_GetDocPermissions(document()));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, PasswordAfterGenerateSave) {
  int num_page_objs = 0;
  {
    ASSERT_TRUE(OpenDocument("encrypted.pdf", "5678"));
    FPDF_PAGE page = LoadPage(0);
    ASSERT_TRUE(page);
    FPDF_PAGEOBJECT path = FPDFPageObj_CreateNewPath(10.0f, 10.0f);
    ASSERT_TRUE(path);
    FPDFPath_MoveTo(path, 100.0f, 100.0f);
    FPDFPage_InsertObject(page, path);
    num_page_objs = FPDFPage_CountObject(page);
    EXPECT_TRUE(FPDFPage_GenerateContent(page));
    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    UnloadPage(page);
  }
  std::string new_file = GetString();
  FPDF_FILEACCESS file_access;
  memset(&file_access, 0, sizeof(file_access));
  file_access.m_FileLen = new_file.size();
  file_access.m_GetBlock = GetBlockFromString;
  file_access.m_Param = &new_file;
  EXPECT_FALSE(FPDF_LoadCustomDocument(&file_access, nullptr));
  struct {
    const char* password;
    const unsigned long permissions;
  } tests[] = {{"1234", 0xFFFFF2C0}, {"5678", 0xFFFFFFFC}};
  for (const auto& test : tests) {
    FPDF_DOCUMENT document =
        FPDF_LoadCustomDocument(&file_access, test.password);
    ASSERT_TRUE(document);
    EXPECT_EQ(test.permissions, FPDF_GetDocPermissions(document));
    EXPECT_EQ(1, FPDF_GetPageCount(document));
    FPDF_PAGE page = FPDF_LoadPage(document, 0);
    ASSERT_TRUE(page);
    EXPECT_EQ(num_page_objs, FPDFPage_CountObject(page));
    FPDF_ClosePage(page);
    FPDF_CloseDocument(document);
  }
}

TEST_F(CPDFSecurityHandlerEmbeddertest, NoPasswordVersion5) {
  ASSERT_FALSE(OpenDocument("bug_644.pdf"));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, BadPasswordVersion5) {
  ASSERT_FALSE(OpenDocument("bug_644.pdf", "tiger"));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, OwnerPasswordVersion5) {
  ASSERT_TRUE(OpenDocument("bug_644.pdf", "a"));
  EXPECT_EQ(0xFFFFFFFC, FPDF_GetDocPermissions(document()));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, UserPasswordVersion5) {
  ASSERT_TRUE(OpenDocument("bug_644.pdf", "b"));
  EXPECT_EQ(0xFFFFFFFC, FPDF_GetDocPermissions(document()));
}
