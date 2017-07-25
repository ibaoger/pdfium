// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_CROSS_REF_AVAIL_CHECKER_H_
#define CORE_FPDFAPI_PARSER_CPDF_CROSS_REF_AVAIL_CHECKER_H_

#include <memory>
#include <queue>
#include <set>

#include "core/fpdfapi/parser/cpdf_data_avail.h"

class CPDF_CrossRefAvailChecker {
 public:
  CPDF_CrossRefAvailChecker(
      const CFX_RetainPtr<IFX_SeekableReadStream>& file_read,
      CPDF_DataAvail::FileAvail* file_avail);
  virtual ~CPDF_CrossRefAvailChecker();

  void Init(FX_FILESIZE last_crossref_offset);

  FX_FILESIZE last_crossref_offset() const { return last_crossref_offset_; }

  CPDF_DataAvail::DocAvailStatus Check(CPDF_DataAvail::DownloadHints* hints);

  CPDF_DataAvail::DocAvailStatus current_status() const {
    return current_status_;
  }

  bool was_read_error() const { return was_read_error_; }
  bool has_unavailable_data() const { return has_unavailable_data_; }

 private:
  class ScopedCheckSession;
  class SeekableReadStreamRetainWrapper;
  enum class State {
    kNotInitialized,
    kCrossRefCheck,
    kCrossRefV4Check,
    kCrossRefV4ItemCheck,
    kCrossRefV4TrailerCheck,
    kCrossRefStreamCheck,
    kDone,
  };

  bool ReadBlock(void* buffer, FX_FILESIZE offset, size_t size);
  FX_FILESIZE GetSize();

  CPDF_SyntaxParser* parser() { return parser_.get(); }

  void ScheduleDownload(FX_FILESIZE offset, size_t size);
  void SetStatus(CPDF_DataAvail::DocAvailStatus status);

  bool CheckReadProblems();
  bool CheckCrossRef();
  bool CheckCrossRefV4();
  bool CheckCrossRefItem();
  bool CheckCrossRefTrailer();
  bool CheckCrossRefStream();

  void AddCrossRefForCheck(FX_FILESIZE crossref_offset);

  bool SetError();

  bool DoStep();

  CFX_RetainPtr<IFX_SeekableReadStream> file_read_;
  CPDF_DataAvail::FileAvail* const file_avail_ = nullptr;

  CPDF_DataAvail::DownloadHints* hints_ = nullptr;
  bool was_read_error_ = false;
  bool has_unavailable_data_ = false;

  FX_FILESIZE last_crossref_offset_ = 0;

  CPDF_DataAvail::DocAvailStatus current_status_ =
      CPDF_DataAvail::DataNotAvailable;

  State current_state_ = State::kNotInitialized;

  FX_FILESIZE current_offset_ = 0;

  std::queue<FX_FILESIZE> cross_refs_for_check_;
  std::set<FX_FILESIZE> registered_crossrefs_;

  std::unique_ptr<CPDF_SyntaxParser> parser_;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_CROSS_REF_AVAIL_CHECKER_H_
