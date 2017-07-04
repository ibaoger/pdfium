// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_SAVERTEST_H_
#define TESTING_SAVERTEST_H_

#include <stdlib.h>

#include <memory>
#include <string>
#include <vector>

#include "public/fpdf_save.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"

class SaverTest : public FPDF_FILEWRITE, public EmbedderTest {
 public:
  SaverTest();

  void ClearString();
  const std::string& GetString() const { return m_String; }

 protected:
  static int GetBlockFromString(void* param,
                                unsigned long pos,
                                unsigned char* buf,
                                unsigned long size);

  void TestSaved(int width, int height, const char* md5);
  void CloseSaved();
  void TestAndCloseSaved(int width, int height, const char* md5);

 private:
  static int WriteBlockCallback(FPDF_FILEWRITE* pFileWrite,
                                const void* data,
                                unsigned long size);

 protected:
  FPDF_DOCUMENT m_SavedDocument;
  FPDF_PAGE m_SavedPage;
  FPDF_FORMHANDLE m_SavedForm;

 private:
  std::string m_String;
};

#endif  // TESTING_SAVERTEST_H_
