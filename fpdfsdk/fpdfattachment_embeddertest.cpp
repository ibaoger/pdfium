// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_attachment.h"
#include "testing/embedder_test.h"

class FPDFAttachmentEmbeddertest : public EmbedderTest {};

TEST_F(FPDFAttachmentEmbeddertest, ExtractAttachments) {
  // Open a file with two attachments.
  ASSERT_TRUE(OpenDocument("embedded_attachments.pdf"));
  EXPECT_EQ(2, FPDFDoc_GetAttachmentCount(document()));

  // Check that the name of the first attachment is correct.
  unsigned long len = FPDFDoc_GetAttachmentName(document(), 0, nullptr, 0);
  std::vector<char> buf(len);
  EXPECT_EQ(12u, FPDFDoc_GetAttachmentName(document(), 0, buf.data(), len));
  EXPECT_STREQ(L"1.txt",
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());

  // Check that the content of the first attachment is correct.
  len = FPDFDoc_GetAttachmentFile(document(), 0, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(4u, FPDFDoc_GetAttachmentFile(document(), 0, buf.data(), len));
  EXPECT_EQ(std::string("test", 4), std::string(buf.data(), 4));

  // Check that the creation date of the first attachment is correct.
  len = FPDFDoc_GetAttachmentDate(document(), 0,
                                  FPDFATTACHMENT_DATETYPE_Creation, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(48u, FPDFDoc_GetAttachmentDate(document(), 0,
                                           FPDFATTACHMENT_DATETYPE_Creation,
                                           buf.data(), len));
  EXPECT_STREQ(L"D:20170712214438-07'00'",
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());

  // Check that the modification date of the first attachment is correct.
  len = FPDFDoc_GetAttachmentDate(
      document(), 0, FPDFATTACHMENT_DATETYPE_Modification, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(34u, FPDFDoc_GetAttachmentDate(document(), 0,
                                           FPDFATTACHMENT_DATETYPE_Modification,
                                           buf.data(), len));
  EXPECT_STREQ(L"D:20160115091400",
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());

  // Check that the checksum of the first attachment is correct.
  len = FPDFDoc_GetAttachmentCheckSum(document(), 0, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(70u, FPDFDoc_GetAttachmentCheckSum(document(), 0, buf.data(), len));
  EXPECT_STREQ(L"<098F6BCD4621D373CADE4E832627B4F6>",
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());

  // Retrieve the second attachment file.
  len = FPDFDoc_GetAttachmentFile(document(), 1, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(5869u, FPDFDoc_GetAttachmentFile(document(), 1, buf.data(), len));

  // Check that the first and the last ten bytes of the file are correct.
  constexpr size_t verify_len = 10;
  EXPECT_EQ(std::string("\x25\x50\x44\x46\x2d\x31\x2e\x35\x0d\x25", verify_len),
            std::string(buf.data(), verify_len));
  EXPECT_EQ(std::string("\x36\x0d\x0a\x25\x25\x45\x4f\x46\x0d\x0a", verify_len),
            std::string(buf.data() + len - verify_len, verify_len));
}
