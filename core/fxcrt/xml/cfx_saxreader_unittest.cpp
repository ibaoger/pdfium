// Copyright 2017 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/xml/cfx_saxreader.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

class CFX_SAXReaderTest : public pdfium::FPDF_Test {
 public:
  bool StartParse(char* str) {
    return reader_.StartParse(IFX_MemoryStream::Create(
                                  reinterpret_cast<uint8_t*>(str), strlen(str)),
                              0, static_cast<uint32_t>(-1),
                              CFX_SaxParseMode_NotSkipSpace) >= 0;
  }

  int32_t ContinueParse() { return reader_.ContinueParse(nullptr); }

 private:
  CFX_SAXReader reader_;
};

TEST_F(CFX_SAXReaderTest, bug_711459) {
  char data[] =
      "&a<tag "
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      ">x;";
  ASSERT_TRUE(StartParse(data));
  int32_t ret;
  do {
    ret = ContinueParse();
  } while (ret >= 0 && ret < 100);
  EXPECT_EQ(100, ret);
}
