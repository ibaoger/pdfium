// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FPDFAPI_PARSER_CPDF_READ_VALIDATOR_H_
#define CORE_FPDFAPI_PARSER_CPDF_READ_VALIDATOR_H_

#include "core/fpdfapi/parser/cpdf_data_avail.h"

class CPDF_ReadValidator : public IFX_SeekableReadStream {
 public:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  void SetDownloadHints(CPDF_DataAvail::DownloadHints* hints) {
    hints_ = hints;
  }

  bool read_error() const { return read_error_; }
  bool has_unavailable_data() const { return has_unavailable_data_; }

  bool has_read_problems() const {
    return read_error() || has_unavailable_data();
  }

  void ResetErrors();

  // IFX_SeekableReadStream overrides:
  bool ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override;
  FX_FILESIZE GetSize() override;

 private:
  CPDF_ReadValidator(const CFX_RetainPtr<IFX_SeekableReadStream>& file_read,
                     CPDF_DataAvail::FileAvail* file_avail);
  ~CPDF_ReadValidator() override;

  void ScheduleDownload(FX_FILESIZE offset, size_t size);

  CFX_RetainPtr<IFX_SeekableReadStream> file_read_;
  CFX_UnownedPtr<CPDF_DataAvail::FileAvail> file_avail_;

  CFX_UnownedPtr<CPDF_DataAvail::DownloadHints> hints_;

  bool read_error_;
  bool has_unavailable_data_;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_READ_VALIDATOR_H_
