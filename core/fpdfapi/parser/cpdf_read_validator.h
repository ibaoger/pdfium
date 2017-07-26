// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FPDFAPI_PARSER_CPDF_READ_VALIDATOR_H_
#define CORE_FPDFAPI_PARSER_CPDF_READ_VALIDATOR_H_

#include <memory>
#include <queue>
#include <set>

#include "core/fpdfapi/parser/cpdf_data_avail.h"

class CPDF_ReadValidator : public IFX_SeekableReadStream {
 public:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  void SetDownloadHints(CPDF_DataAvail::DownloadHints* hints) {
    hints_ = hints;
  }

  bool was_read_error() const { return was_read_error_; }
  bool has_unavailable_data() const { return has_unavailable_data_; }

  bool has_read_problems() const {
    return was_read_error() || has_unavailable_data();
  }

  void ResetErrors();

  // IFX_SeekableReadStream overrides:
  bool ReadBlock(void* buffer, FX_FILESIZE offset, size_t size);
  FX_FILESIZE GetSize();

 private:
  CPDF_ReadValidator(const CFX_RetainPtr<IFX_SeekableReadStream>& file_read,
                     CPDF_DataAvail::FileAvail* file_avail);
  ~CPDF_ReadValidator() override;

  void ScheduleDownload(FX_FILESIZE offset, size_t size);

  CFX_RetainPtr<IFX_SeekableReadStream> file_read_;
  CPDF_DataAvail::FileAvail* const file_avail_ = nullptr;

  CPDF_DataAvail::DownloadHints* hints_ = nullptr;
  bool was_read_error_ = false;
  bool has_unavailable_data_ = false;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_READ_VALIDATOR_H_
