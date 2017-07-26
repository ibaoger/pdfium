// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_read_validator.h"

#include <algorithm>

namespace {
constexpr FX_FILESIZE kAlignBlockValue = 512;

FX_FILESIZE AlignDown(FX_FILESIZE offset) {
  if (offset < 0)
    return 0;
  return offset - offset % kAlignBlockValue;
}

FX_FILESIZE AlignUp(FX_FILESIZE offset) {
  FX_SAFE_FILESIZE safe_result = AlignDown(offset);
  safe_result += kAlignBlockValue;
  if (!safe_result.IsValid())
    return offset;
  return safe_result.ValueOrDie();
}

}  // namespace

CPDF_ReadValidator::CPDF_ReadValidator(
    const CFX_RetainPtr<IFX_SeekableReadStream>& file_read,
    CPDF_DataAvail::FileAvail* file_avail)
    : file_read_(file_read), file_avail_(file_avail) {
  ASSERT(file_read_);
}

CPDF_ReadValidator::~CPDF_ReadValidator() {}

void CPDF_ReadValidator::ResetErrors() {
  was_read_error_ = false;
  has_unavailable_data_ = false;
}

bool CPDF_ReadValidator::ReadBlock(void* buffer,
                                   FX_FILESIZE offset,
                                   size_t size) {
  // correct values checks:
  if (!pdfium::base::IsValueInRangeForNumericType<uint32_t>(size)) {
    return false;
  }

  FX_SAFE_FILESIZE end_offset = offset;
  end_offset += size;
  if (!end_offset.IsValid() || end_offset.ValueOrDie() > GetSize()) {
    return false;
  }

  if (!file_avail_ ||
      file_avail_->IsDataAvail(offset, static_cast<uint32_t>(size))) {
    if (file_read_->ReadBlock(buffer, offset, size))
      return true;
    was_read_error_ = true;
  }
  has_unavailable_data_ = true;
  ScheduleDownload(offset, size);
  return false;
}

FX_FILESIZE CPDF_ReadValidator::GetSize() {
  return file_read_->GetSize();
}

void CPDF_ReadValidator::ScheduleDownload(FX_FILESIZE offset, size_t size) {
  if (!hints_ || !size)
    return;
  const FX_FILESIZE start_segment_offset = AlignDown(offset);
  FX_SAFE_FILESIZE end_segment_offset = offset;
  end_segment_offset += size;
  if (!end_segment_offset.IsValid()) {
    return;
  }
  end_segment_offset =
      std::min(GetSize(), AlignUp(end_segment_offset.ValueOrDie()));

  FX_SAFE_UINT32 segment_size = end_segment_offset;
  segment_size -= start_segment_offset;
  if (!segment_size.IsValid()) {
    return;
  }
  hints_->AddSegment(start_segment_offset, segment_size.ValueOrDie());
}
