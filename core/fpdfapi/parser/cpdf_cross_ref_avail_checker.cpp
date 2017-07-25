// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_cross_ref_avail_checker.h"

#include <algorithm>
#include <vector>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_syntax_parser.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"

namespace {
constexpr FX_FILESIZE kAlignBlockValue = 512;

constexpr char kCrossRefKeyword[] = "xref";
constexpr char kTrailerKeyword[] = "trailer";
constexpr char kPrevCrossRefFieldKey[] = "Prev";
constexpr char kTypeFieldKey[] = "Type";
constexpr char kPrevCrossRefStreamOffsetFieldKey[] = "XRefStm";
constexpr char kXRefKeyword[] = "XRef";

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

class CPDF_CrossRefAvailChecker::SeekableReadStreamRetainWrapper
    : public IFX_SeekableReadStream {
 public:
  explicit SeekableReadStreamRetainWrapper(CPDF_CrossRefAvailChecker* owner)
      : owner_(owner) {}

  bool ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override {
    return owner_->ReadBlock(buffer, offset, size);
  }

  FX_FILESIZE GetSize() override { return owner_->GetSize(); }

 private:
  CPDF_CrossRefAvailChecker* owner_;
};

class CPDF_CrossRefAvailChecker::ScopedCheckSession {
 public:
  ScopedCheckSession(CPDF_CrossRefAvailChecker* owner,
                     CPDF_DataAvail::DownloadHints* hints)
      : owner_(owner) {
    ASSERT(!owner_->hints_);
    owner_->hints_ = hints;
    owner_->was_read_error_ = false;
    owner_->has_unavailable_data_ = false;
  }
  ~ScopedCheckSession() { owner_->hints_ = nullptr; }

 private:
  CPDF_CrossRefAvailChecker* owner_;
};

CPDF_CrossRefAvailChecker::CPDF_CrossRefAvailChecker(
    const CFX_RetainPtr<IFX_SeekableReadStream>& file_read,
    CPDF_DataAvail::FileAvail* file_avail)
    : file_read_(file_read),
      file_avail_(file_avail),
      parser_(pdfium::MakeUnique<CPDF_SyntaxParser>()) {
  ASSERT(file_read_);
  parser_->InitParser(pdfium::MakeRetain<SeekableReadStreamRetainWrapper>(this),
                      0);
}

CPDF_CrossRefAvailChecker::~CPDF_CrossRefAvailChecker() {
  parser_.reset();
}

void CPDF_CrossRefAvailChecker::SetStatus(
    CPDF_DataAvail::DocAvailStatus status) {
  current_status_ = status;
}

void CPDF_CrossRefAvailChecker::Init(FX_FILESIZE last_crossref_offset) {
  ASSERT(current_state_ == State::kNotInitialized);
  last_crossref_offset_ = last_crossref_offset;
  AddCrossRefForCheck(last_crossref_offset);
  current_state_ = State::kCrossRefCheck;
}

CPDF_DataAvail::DocAvailStatus CPDF_CrossRefAvailChecker::Check(
    CPDF_DataAvail::DownloadHints* hints) {
  if (current_status_ == CPDF_DataAvail::DataAvailable)
    return CPDF_DataAvail::DataAvailable;

  const ScopedCheckSession check_session(this, hints);
  while (DoStep()) {
    // Nothing
  }
  return current_status();
}

bool CPDF_CrossRefAvailChecker::DoStep() {
  switch (current_state_) {
    case State::kNotInitialized:
      return SetError();
    case State::kCrossRefCheck:
      return CheckCrossRef();
    case State::kCrossRefV4Check:
      return CheckCrossRefV4();
    case State::kCrossRefV4ItemCheck:
      return CheckCrossRefItem();
    case State::kCrossRefV4TrailerCheck:
      return CheckCrossRefTrailer();
    case State::kCrossRefStreamCheck:
      return CheckCrossRefStream();
    case State::kDone:
      return false;
    default: {
      ASSERT(false);  // NOTREACHED();
      return SetError();
    }
  }
}

bool CPDF_CrossRefAvailChecker::CheckReadProblems() {
  if (was_read_error()) {
    ASSERT(has_unavailable_data());
    SetError();
    return true;
  }
  if (has_unavailable_data()) {
    return true;
  }
  return false;
}

bool CPDF_CrossRefAvailChecker::CheckCrossRef() {
  if (cross_refs_for_check_.empty()) {
    // All cross refs were checked.
    current_state_ = State::kDone;
    SetStatus(CPDF_DataAvail::DataAvailable);
    return true;
  }
  current_offset_ = cross_refs_for_check_.front();
  parser()->SetPos(current_offset_);

  bool is_number = false;
  parser()->GetNextWord(&is_number);
  if (CheckReadProblems()) {
    return false;
  }
  // select cross ref type
  if (is_number) {
    current_state_ = State::kCrossRefStreamCheck;
  } else {
    current_state_ = State::kCrossRefV4Check;
  }
  cross_refs_for_check_.pop();
  return true;
}

bool CPDF_CrossRefAvailChecker::CheckCrossRefV4() {
  parser()->SetPos(current_offset_);

  const CFX_ByteString keyword = parser()->GetKeyword();
  if (CheckReadProblems())
    return false;

  if (keyword != kCrossRefKeyword) {
    return SetError();
  }

  current_state_ = State::kCrossRefV4ItemCheck;
  current_offset_ = parser()->GetPos();
  return true;
}

bool CPDF_CrossRefAvailChecker::CheckCrossRefItem() {
  parser()->SetPos(current_offset_);
  const CFX_ByteString keyword = parser()->GetKeyword();
  if (CheckReadProblems())
    return false;
  if (keyword.IsEmpty()) {
    return SetError();
  }
  if (keyword == kTrailerKeyword) {
    current_state_ = State::kCrossRefV4TrailerCheck;
  }
  // Got to next item.
  current_offset_ = parser()->GetPos();
  return true;
}

bool CPDF_CrossRefAvailChecker::CheckCrossRefTrailer() {
  parser()->SetPos(current_offset_);

  std::unique_ptr<CPDF_Dictionary> trailer =
      ToDictionary(parser()->GetObject(nullptr, 0, 0, true));
  if (CheckReadProblems())
    return false;

  if (!trailer) {
    return SetError();
  }

  const int32_t xrefpos =
      GetDirectInteger(trailer.get(), kPrevCrossRefFieldKey);
  if (xrefpos &&
      pdfium::base::IsValueInRangeForNumericType<FX_FILESIZE>(xrefpos)) {
    AddCrossRefForCheck(static_cast<FX_FILESIZE>(xrefpos));
  }

  const int32_t stream_xref_offset =
      GetDirectInteger(trailer.get(), kPrevCrossRefStreamOffsetFieldKey);
  if (stream_xref_offset &&
      pdfium::base::IsValueInRangeForNumericType<FX_FILESIZE>(
          stream_xref_offset)) {
    AddCrossRefForCheck(static_cast<FX_FILESIZE>(stream_xref_offset));
  }

  // Goto check next crossref
  current_state_ = State::kCrossRefCheck;
  return true;
}

bool CPDF_CrossRefAvailChecker::CheckCrossRefStream() {
  parser()->SetPos(current_offset_);

  bool is_number = false;
  CFX_ByteString word = parser()->GetNextWord(&is_number);
  if (CheckReadProblems()) {
    return false;
  }

  if (!is_number || word.IsEmpty()) {
    return SetError();
  }

  word = parser()->GetNextWord(&is_number);
  if (CheckReadProblems()) {
    return false;
  }
  if (!is_number || word.IsEmpty()) {
    return SetError();
  }

  const CFX_ByteString obj_keyword = parser()->GetKeyword();
  if (CheckReadProblems()) {
    return false;
  }

  if (obj_keyword != "obj") {
    return SetError();
  }

  std::unique_ptr<CPDF_Object> object =
      parser()->GetObject(nullptr, 0, 0, false);
  if (CheckReadProblems()) {
    return false;
  }

  CPDF_Dictionary* dict = object ? object->GetDict() : nullptr;
  if (!dict) {
    return SetError();
  }

  CPDF_Name* type_name = ToName(dict->GetObjectFor(kTypeFieldKey));
  if (type_name && type_name->GetString() == kXRefKeyword) {
    const int32_t xrefpos = GetDirectInteger(dict, kPrevCrossRefFieldKey);
    if (xrefpos &&
        pdfium::base::IsValueInRangeForNumericType<FX_FILESIZE>(xrefpos)) {
      AddCrossRefForCheck(static_cast<FX_FILESIZE>(xrefpos));
    }
  }
  // Goto check next crossref
  current_state_ = State::kCrossRefCheck;
  return true;
}

void CPDF_CrossRefAvailChecker::AddCrossRefForCheck(
    FX_FILESIZE crossref_offset) {
  if (registered_crossrefs_.count(crossref_offset))
    return;
  cross_refs_for_check_.push(crossref_offset);
  registered_crossrefs_.insert(crossref_offset);
}

bool CPDF_CrossRefAvailChecker::SetError() {
  SetStatus(CPDF_DataAvail::DataError);
  return false;
}

bool CPDF_CrossRefAvailChecker::ReadBlock(void* buffer,
                                          FX_FILESIZE offset,
                                          size_t size) {
  if (!pdfium::base::IsValueInRangeForNumericType<uint32_t>(size))
    return false;
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

FX_FILESIZE CPDF_CrossRefAvailChecker::GetSize() {
  return file_read_->GetSize();
}

void CPDF_CrossRefAvailChecker::ScheduleDownload(FX_FILESIZE offset,
                                                 size_t size) {
  if (!hints_)
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
