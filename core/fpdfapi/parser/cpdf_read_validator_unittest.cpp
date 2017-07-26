// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_read_validator.h"

#include <utility>
#include <vector>

#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/fx_stream.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {
static const uint32_t kTestDataSize = 64 * 1024 - 467;

inline std::pair<FX_FILESIZE, FX_FILESIZE> MakeRange(uint32_t start,
                                                     uint32_t end) {
  return std::pair<FX_FILESIZE, FX_FILESIZE>(start, end);
}

class MockFileAvail : public CPDF_DataAvail::FileAvail {
 public:
  MockFileAvail() : available_range_(0, 0) {}
  ~MockFileAvail() override {}

  bool IsDataAvail(FX_FILESIZE offset, uint32_t size) override {
    return available_range_.first <= offset &&
           available_range_.second >= static_cast<FX_FILESIZE>(offset + size);
  }

  void SetAvailableRange(const std::pair<FX_FILESIZE, FX_FILESIZE>& range) {
    available_range_ = range;
  }

  void SetAvailableRange(uint32_t start, uint32_t end) {
    SetAvailableRange(MakeRange(start, end));
  }

 private:
  std::pair<FX_FILESIZE, FX_FILESIZE> available_range_;
};

class MockDownloadHints : public CPDF_DataAvail::DownloadHints {
 public:
  MockDownloadHints() : last_requested_range_(0, 0) {}
  ~MockDownloadHints() override {}

  void AddSegment(FX_FILESIZE offset, uint32_t size) override {
    last_requested_range_.first = offset;
    last_requested_range_.second = offset + size;
  }

  std::pair<FX_FILESIZE, FX_FILESIZE> GetLastRequstedRange() const {
    return last_requested_range_;
  }

  void Reset() { last_requested_range_ = MakeRange(0, 0); }

 private:
  std::pair<FX_FILESIZE, FX_FILESIZE> last_requested_range_;
};

class InvalidReader : public IFX_SeekableReadStream {
 public:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // IFX_SeekableReadStream overrides:
  bool ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override {
    return false;
  }
  FX_FILESIZE GetSize() override { return kTestDataSize; }

 private:
  InvalidReader() {}
  ~InvalidReader() override {}
};

}  // namespace

TEST(cpdf_read_validator, UnavailableData) {
  std::vector<uint8_t> test_data(kTestDataSize);
  auto file = pdfium::MakeRetain<CFX_MemoryStream>(test_data.data(),
                                                   test_data.size(), false);
  MockFileAvail file_avail;
  auto validator = pdfium::MakeRetain<CPDF_ReadValidator>(file, &file_avail);

  std::vector<uint8_t> read_buffer(100);
  EXPECT_FALSE(
      validator->ReadBlock(read_buffer.data(), 5000, read_buffer.size()));

  EXPECT_FALSE(validator->was_read_error());
  EXPECT_TRUE(validator->has_unavailable_data());

  validator->ResetErrors();

  file_avail.SetAvailableRange(5000, 5000 + read_buffer.size());

  EXPECT_TRUE(
      validator->ReadBlock(read_buffer.data(), 5000, read_buffer.size()));
  EXPECT_FALSE(validator->was_read_error());
  EXPECT_FALSE(validator->has_unavailable_data());
}

TEST(cpdf_read_validator, UnavailableDataWithHints) {
  std::vector<uint8_t> test_data(kTestDataSize);
  auto file = pdfium::MakeRetain<CFX_MemoryStream>(test_data.data(),
                                                   test_data.size(), false);
  MockFileAvail file_avail;
  auto validator = pdfium::MakeRetain<CPDF_ReadValidator>(file, &file_avail);

  MockDownloadHints hints;
  validator->SetDownloadHints(&hints);

  std::vector<uint8_t> read_buffer(100);

  EXPECT_FALSE(
      validator->ReadBlock(read_buffer.data(), 5000, read_buffer.size()));
  EXPECT_FALSE(validator->was_read_error());
  EXPECT_TRUE(validator->has_unavailable_data());

  // Requested range should be enlarged and aligned.
  EXPECT_EQ(MakeRange(4608, 5120), hints.GetLastRequstedRange());

  file_avail.SetAvailableRange(hints.GetLastRequstedRange());
  hints.Reset();

  validator->ResetErrors();
  EXPECT_TRUE(
      validator->ReadBlock(read_buffer.data(), 5000, read_buffer.size()));
  // No new request on already available data.
  EXPECT_EQ(MakeRange(0, 0), hints.GetLastRequstedRange());
  EXPECT_FALSE(validator->was_read_error());
  EXPECT_FALSE(validator->has_unavailable_data());

  validator->ResetErrors();
  // Try read unavailable data at file end.
  EXPECT_FALSE(validator->ReadBlock(read_buffer.data(),
                                    validator->GetSize() - read_buffer.size(),
                                    read_buffer.size()));
  // Should not enlarge request at file end.
  EXPECT_EQ(validator->GetSize(), hints.GetLastRequstedRange().second);
  EXPECT_FALSE(validator->was_read_error());
  EXPECT_TRUE(validator->has_unavailable_data());
}

TEST(cpdf_read_validator, ReadError) {
  auto file = pdfium::MakeRetain<InvalidReader>();
  auto validator = pdfium::MakeRetain<CPDF_ReadValidator>(file, nullptr);

  static const uint32_t kBufferSize = 3 * 1000;
  std::vector<uint8_t> buffer(kBufferSize);

  EXPECT_FALSE(validator->ReadBlock(buffer.data(), 5000, 100));
  EXPECT_TRUE(validator->was_read_error());
  EXPECT_TRUE(validator->has_unavailable_data());
}
