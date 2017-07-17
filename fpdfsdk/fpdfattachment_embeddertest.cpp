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
  ASSERT_EQ(4u, FPDFDoc_GetAttachmentFile(document(), 0, buf.data(), len));
  EXPECT_EQ(std::string("test", 4), std::string(buf.data(), 4));

  // Check that a non-existent key does not exist.
  EXPECT_FALSE(FPDFDoc_HasAttachmentKey(document(), 0,
                                        GetFPDFWideString(L"none").get()));

  // Check that the string value of a non-string dictionary entry is empty.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> size_key =
      GetFPDFWideString(L"Size");
  EXPECT_EQ(FPDF_OBJECT_NUMBER,
            FPDFDoc_GetAttachmentValueType(document(), 0, size_key.get()));
  EXPECT_EQ(2u, FPDFDoc_GetAttachmentStringValue(document(), 0, size_key.get(),
                                                 nullptr, 0));

  // Check that the creation date of the first attachment is correct.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> date_key =
      GetFPDFWideString(L"CreationDate");
  len = FPDFDoc_GetAttachmentStringValue(document(), 0, date_key.get(), nullptr,
                                         0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(48u, FPDFDoc_GetAttachmentStringValue(document(), 0, date_key.get(),
                                                  buf.data(), len));
  EXPECT_STREQ(L"D:20170712214438-07'00'",
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());

  // Retrieve the second attachment file.
  len = FPDFDoc_GetAttachmentFile(document(), 1, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(5869u, FPDFDoc_GetAttachmentFile(document(), 1, buf.data(), len));

  // Check that the calculated checksum of the file data matches expectation.
  std::wstring checksum = L"72afcddedf554dda63c0c88e06f1ce18";
  const std::string generated_checksum =
      GenerateMD5Base16(reinterpret_cast<uint8_t*>(buf.data()), len);
  EXPECT_EQ(checksum,
            std::wstring(generated_checksum.begin(), generated_checksum.end()));

  // Check that the stored checksum matches expectation.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> checksum_key =
      GetFPDFWideString(L"CheckSum");
  len = FPDFDoc_GetAttachmentStringValue(document(), 1, checksum_key.get(),
                                         nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(70u, FPDFDoc_GetAttachmentStringValue(
                     document(), 1, checksum_key.get(), buf.data(), len));
  transform(checksum.begin(), checksum.end(), checksum.begin(), towupper);
  EXPECT_EQ(L"<" + checksum + L">",
            GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data())));
}
