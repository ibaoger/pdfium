// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_cross_ref_avail_checker.h"

#include <memory>
#include <string>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

namespace {
// Provide a way to read test data from a buffer instead of a file.
class TestBufferRead : public IFX_SeekableReadStream {
 public:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // IFX_SeekableReadStream:
  bool ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override {
    if (offset < 0 || offset + size > total_size_)
      return false;

    memcpy(buffer, buffer_ + offset, size);
    return true;
  }

  FX_FILESIZE GetSize() override { return (FX_FILESIZE)total_size_; };

 protected:
  TestBufferRead(const unsigned char* buffer_in, size_t buf_size)
      : buffer_(buffer_in), total_size_(buf_size) {}

  const unsigned char* buffer_;
  size_t total_size_;
};

std::unique_ptr<CPDF_CrossRefAvailChecker> MakeCheckerForBuffer(
    const unsigned char* buffer,
    size_t buffer_size,
    CPDF_DataAvail::FileAvail* file_avail) {
  return pdfium::MakeUnique<CPDF_CrossRefAvailChecker>(
      pdfium::MakeRetain<TestBufferRead>(buffer, buffer_size), file_avail);
}

}  // namespace

TEST(cpdf_cross_ref_avail_checker, CheckCrossRefV4) {
  const unsigned char xref_table[] =
      "xref \n"
      "0 6 \n"
      "0000000003 65535 f \n"
      "0000000017 00000 n \n"
      "0000000081 00000 n \n"
      "0000000000 00007 f \n"
      "0000000331 00000 n \n"
      "0000000409 00000 n \n"
      "trailer\n"
      "<</Root 14 0 R/ID "
      "[<afbb0f593c2d2aea5b519cb61da1c17b><4f9bb2e7978401808f8f1f2a75c322c8>]"
      "/Info 15 0 R/Size 16>>";

  std::unique_ptr<CPDF_CrossRefAvailChecker> checker =
      MakeCheckerForBuffer(xref_table, FX_ArraySize(xref_table), nullptr);
  checker->Init(0);
  EXPECT_EQ(CPDF_DataAvail::DataAvailable, checker->Check(nullptr));
}

TEST(cpdf_cross_ref_avail_checker, CheckCrossRefStrem) {
  const unsigned char xref_stream[] =
      "16 0 obj\n"
      "<</Filter /FlateDecode>>"
      " stream \n"
      "STREAM DATA STREAM DATA STREAM DATA\n"
      "endstream\n"
      "endobj\n";

  std::unique_ptr<CPDF_CrossRefAvailChecker> checker =
      MakeCheckerForBuffer(xref_stream, FX_ArraySize(xref_stream), nullptr);
  checker->Init(0);
  EXPECT_EQ(CPDF_DataAvail::DataAvailable, checker->Check(nullptr));
}

TEST(cpdf_cross_ref_avail_checker, IncorrectStartOffset) {
  const unsigned char xref_stream[] =
      "16 0 obj\n"
      "<</Filter /FlateDecode>>"
      " stream \n"
      "STREAM DATA STREAM DATA STREAM DATA\n"
      "endstream\n"
      "endobj\n";

  std::unique_ptr<CPDF_CrossRefAvailChecker> checker =
      MakeCheckerForBuffer(xref_stream, FX_ArraySize(xref_stream), nullptr);
  checker->Init(70000);
  EXPECT_EQ(CPDF_DataAvail::DataError, checker->Check(nullptr));
}

TEST(cpdf_cross_ref_avail_checker, IncorrectPrevOffset) {
  const unsigned char xref_stream[] =
      "16 0 obj\n"
      "<</Type /XRef /Prev 70000>>"
      " stream \n"
      "STREAM DATA STREAM DATA STREAM DATA\n"
      "endstream\n"
      "endobj\n";
  std::unique_ptr<CPDF_CrossRefAvailChecker> checker =
      MakeCheckerForBuffer(xref_stream, FX_ArraySize(xref_stream), nullptr);
  checker->Init(0);
  EXPECT_EQ(CPDF_DataAvail::DataError, checker->Check(nullptr));
}

TEST(cpdf_cross_ref_avail_checker, IncorrectPrevStreamOffset) {
  const unsigned char xref_table[] =
      "xref \n"
      "0 6 \n"
      "0000000003 65535 f \n"
      "0000000017 00000 n \n"
      "0000000081 00000 n \n"
      "0000000000 00007 f \n"
      "0000000331 00000 n \n"
      "0000000409 00000 n \n"
      "trailer\n"
      "<</Root 14 0 R/ID "
      "[<afbb0f593c2d2aea5b519cb61da1c17b><4f9bb2e7978401808f8f1f2a75c322c8>]"
      "/Info 15 0 R/Size 16 /XRefStm 70000>>";
  std::unique_ptr<CPDF_CrossRefAvailChecker> checker =
      MakeCheckerForBuffer(xref_table, FX_ArraySize(xref_table), nullptr);
  checker->Init(0);
  EXPECT_EQ(CPDF_DataAvail::DataError, checker->Check(nullptr));
}

TEST(cpdf_cross_ref_avail_checker, IncorrectData) {
  const unsigned char incorrect_data[] =
      "fiajaoilf w9ifaoihwoiafhja wfijaofijoiaw fhj oiawhfoiah "
      "wfoihoiwfghouiafghwoigahfi";
  std::unique_ptr<CPDF_CrossRefAvailChecker> checker = MakeCheckerForBuffer(
      incorrect_data, FX_ArraySize(incorrect_data), nullptr);
  checker->Init(10);
  EXPECT_EQ(CPDF_DataAvail::DataError, checker->Check(nullptr));
}

TEST(cpdf_cross_ref_avail_checker, ThreeCrossRefV4) {
  char int_buffer[100];
  int prev_offset = 0;
  int cur_offset = 0;
  std::string table = "pdf blah blah blah\n";
  prev_offset = cur_offset;
  cur_offset = static_cast<int>(table.size());
  table +=
      "xref \n"
      "0 6 \n"
      "0000000003 65535 f \n"
      "trailer\n"
      "<</Root 14 0 R/ID "
      "[<afbb0f593c2d2aea5b519cb61da1c17b><4f9bb2e7978401808f8f1f2a75c322c8>]"
      "/Info 15 0 R/Size 16>>\n";
  table += "Dummy Data jgwhughouiwbahng";
  prev_offset = cur_offset;
  cur_offset = static_cast<int>(table.size());
  table += std::string(
               "xref \n"
               "0 6 \n"
               "0000000003 65535 f \n"
               "trailer\n"
               "<</Root 14 0 R/ID "
               "[<afbb0f593c2d2aea5b519cb61da1c17b><"
               "4f9bb2e7978401808f8f1f2a75c322c8>]"
               "/Info 15 0 R/Size 16"
               "/Prev ") +
           FXSYS_itoa(prev_offset, int_buffer, 10) + ">>\n";
  table += "More Dummy Data jgwhughouiwbahng";
  prev_offset = cur_offset;
  cur_offset = static_cast<int>(table.size());
  table += std::string(
               "xref \n"
               "0 6 \n"
               "0000000003 65535 f \n"
               "trailer\n"
               "<</Root 14 0 R/ID "
               "[<afbb0f593c2d2aea5b519cb61da1c17b><"
               "4f9bb2e7978401808f8f1f2a75c322c8>]"
               "/Info 15 0 R/Size 16"
               "/Prev ") +
           FXSYS_itoa(prev_offset, int_buffer, 10) + ">>\n";
  std::unique_ptr<CPDF_CrossRefAvailChecker> checker =
      MakeCheckerForBuffer(reinterpret_cast<const unsigned char*>(table.data()),
                           table.size(), nullptr);
  checker->Init(cur_offset);
  EXPECT_EQ(CPDF_DataAvail::DataAvailable, checker->Check(nullptr));
}

TEST(cpdf_cross_ref_avail_checker, ThreeCrossRefV5) {
  char int_buffer[100];
  int prev_offset = 0;
  int cur_offset = 0;
  std::string table = "pdf blah blah blah\n";
  prev_offset = cur_offset;
  cur_offset = static_cast<int>(table.size());
  table +=
      "16 0 obj\n"
      "<</Type /XRef>>"
      " stream \n"
      "STREAM DATA STREAM DATA STREAM DATA ahfcuabfkuabfu\n"
      "endstream\n"
      "endobj\n";
  table += "Dummy Data jgwhughouiwbahng";

  prev_offset = cur_offset;
  cur_offset = static_cast<int>(table.size());
  table += std::string(
               "55 0 obj\n"
               "<</Type /XRef /Prev ") +
           FXSYS_itoa(prev_offset, int_buffer, 10) +
           ">>"
           " stream \n"
           "STREAM DATA STREAM DATA STREAM DATA\n"
           "endstream\n"
           "endobj\n";
  table += "More Dummy Data jgwhughouiwbahng";
  prev_offset = cur_offset;
  cur_offset = static_cast<int>(table.size());
  table += std::string(
               "88 0 obj\n"
               "<</Type /XRef /NNNN /Prev ") +
           FXSYS_itoa(prev_offset, int_buffer, 10) +
           ">>"
           " stream \n"
           "STREAM DATA STREAM DATA STREAM DATA favav\n"
           "endstream\n"
           "endobj\n";
  std::unique_ptr<CPDF_CrossRefAvailChecker> checker =
      MakeCheckerForBuffer(reinterpret_cast<const unsigned char*>(table.data()),
                           table.size(), nullptr);
  checker->Init(cur_offset);
  EXPECT_EQ(CPDF_DataAvail::DataAvailable, checker->Check(nullptr));
}

TEST(cpdf_cross_ref_avail_checker, Mixed) {
  char int_buffer[100];
  std::string table = "pdf blah blah blah\n";

  const int first_v5_table_offset = static_cast<int>(table.size());
  table +=
      "16 0 obj\n"
      "<</Type /XRef>>"
      " stream \n"
      "STREAM DATA STREAM DATA STREAM DATA ahfcuabfkuabfu\n"
      "endstream\n"
      "endobj\n";
  table += "Dummy Data jgwhughouiwbahng";

  const int second_v4_table_offset = static_cast<int>(table.size());
  table += std::string(
               "xref \n"
               "0 6 \n"
               "0000000003 65535 f \n"
               "trailer\n"
               "<</Root 14 0 R/ID "
               "[<afbb0f593c2d2aea5b519cb61da1c17b><"
               "4f9bb2e7978401808f8f1f2a75c322c8>]"
               "/Info 15 0 R/Size 16"
               "/Prev ") +
           FXSYS_itoa(first_v5_table_offset, int_buffer, 10) + ">>\n";
  table += "More Dummy Data jgwhughouiwbahng";

  const int last_v4_table_offset = static_cast<int>(table.size());
  table += std::string(
               "xref \n"
               "0 6 \n"
               "0000000003 65535 f \n"
               "trailer\n"
               "<</Root 14 0 R/ID "
               "[<afbb0f593c2d2aea5b519cb61da1c17b><"
               "4f9bb2e7978401808f8f1f2a75c322c8>]"
               "/Info 15 0 R/Size 16"
               "/Prev ") +
           FXSYS_itoa(second_v4_table_offset, int_buffer, 10) + " /XRefStm " +
           FXSYS_itoa(first_v5_table_offset, int_buffer, 10) + ">>\n";
  std::unique_ptr<CPDF_CrossRefAvailChecker> checker =
      MakeCheckerForBuffer(reinterpret_cast<const unsigned char*>(table.data()),
                           table.size(), nullptr);
  checker->Init(last_v4_table_offset);
  EXPECT_EQ(CPDF_DataAvail::DataAvailable, checker->Check(nullptr));
}
