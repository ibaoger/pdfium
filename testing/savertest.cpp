// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/savertest.h"

#include <string>

SaverTest::SaverTest() {
  FPDF_FILEWRITE::version = 1;
  FPDF_FILEWRITE::WriteBlock = WriteBlockCallback;
}

void SaverTest::ClearString() {
  m_String.clear();
}

// static
int SaverTest::WriteBlockCallback(FPDF_FILEWRITE* pFileWrite,
                                  const void* data,
                                  unsigned long size) {
  SaverTest* pThis = static_cast<SaverTest*>(pFileWrite);
  pThis->m_String.append(static_cast<const char*>(data), size);
  return 1;
}

// static
int SaverTest::GetBlockFromString(void* param,
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

void SaverTest::TestSaved(int width, int height, const char* md5) {
  FPDF_FILEACCESS file_access;
  memset(&file_access, 0, sizeof(file_access));
  file_access.m_FileLen = m_String.size();
  file_access.m_GetBlock = GetBlockFromString;
  file_access.m_Param = &m_String;

  m_SavedDocument = FPDF_LoadCustomDocument(&file_access, nullptr);
  ASSERT_TRUE(m_SavedDocument);
  EXPECT_EQ(1, FPDF_GetPageCount(m_SavedDocument));
  m_SavedPage = FPDF_LoadPage(m_SavedDocument, 0);
  ASSERT_TRUE(m_SavedPage);
  FPDF_BITMAP new_bitmap = RenderPage(m_SavedPage);
  CompareBitmap(new_bitmap, width, height, md5);
  FPDFBitmap_Destroy(new_bitmap);
}

void SaverTest::TestAndCloseSaved(int width, int height, const char* md5) {
  TestSaved(width, height, md5);
  FPDF_ClosePage(m_SavedPage);
  FPDF_CloseDocument(m_SavedDocument);
}
