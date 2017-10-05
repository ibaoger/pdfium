// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_cross_ref_avail.h"

#include <algorithm>
#include <vector>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_read_validator.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_syntax_parser.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"

namespace {

constexpr char kCrossRefKeyword[] = "xref";
constexpr char kTrailerKeyword[] = "trailer";
constexpr char kPrevCrossRefFieldKey[] = "Prev";
constexpr char kTypeFieldKey[] = "Type";
constexpr char kPrevCrossRefStreamOffsetFieldKey[] = "XRefStm";
constexpr char kXRefKeyword[] = "XRef";
constexpr char kEncryptKey[] = "Encrypt";

}  // namespace

CPDF_CrossRefAvail::CPDF_CrossRefAvail(CPDF_SyntaxParser* parser,
                                       FX_FILESIZE last_crossref_offset)
    : parser_(parser), last_crossref_offset_(last_crossref_offset) {
  ASSERT(parser_);
  AddCrossRefForCheck(last_crossref_offset);
}

CPDF_CrossRefAvail::~CPDF_CrossRefAvail() {}

CPDF_DataAvail::DocAvailStatus CPDF_CrossRefAvail::CheckAvail() {
  if (current_status_ == CPDF_DataAvail::DataAvailable)
    return CPDF_DataAvail::DataAvailable;

  const CPDF_ReadValidator::Session read_session(GetValidator().Get());
  while (DoStep()) {
    ASSERT(!GetValidator()->has_read_problems());
  }
  return current_status_;
}

bool CPDF_CrossRefAvail::DoStep() {
  switch (current_state_) {
    case State::kCrossRefCheck:
      return CheckCrossRef();
    case State::kCrossRefV4ItemCheck:
      return CheckCrossRefV4Item();
    case State::kCrossRefV4TrailerCheck:
      return CheckCrossRefV4Trailer();
    case State::kDone:
      return false;
    default: {
      NOTREACHED();
      return SetError();
    }
  }
}

bool CPDF_CrossRefAvail::CheckReadProblems() {
  if (GetValidator()->read_error()) {
    SetError();
    return true;
  }
  return GetValidator()->has_unavailable_data();
}

bool CPDF_CrossRefAvail::CheckCrossRef() {
  if (cross_refs_for_check_.empty()) {
    // All cross refs were checked.
    current_state_ = State::kDone;
    current_status_ = CPDF_DataAvail::DataAvailable;
    return true;
  }
  parser_->SetPos(cross_refs_for_check_.front());

  const ByteString first_word = parser_->PeekNextWord(nullptr);
  if (CheckReadProblems())
    return false;

  const bool result = (first_word == kCrossRefKeyword) ? CheckCrossRefV4()
                                                       : CheckCrossRefStream();

  if (result)
    cross_refs_for_check_.pop();

  return result;
}

bool CPDF_CrossRefAvail::CheckCrossRefV4() {
  const ByteString keyword = parser_->GetKeyword();
  if (CheckReadProblems())
    return false;

  if (keyword != kCrossRefKeyword)
    return SetError();

  current_state_ = State::kCrossRefV4ItemCheck;
  current_offset_ = parser_->GetPos();
  return true;
}

bool CPDF_CrossRefAvail::CheckCrossRefV4Item() {
  parser_->SetPos(current_offset_);
  const ByteString keyword = parser_->GetKeyword();
  if (CheckReadProblems())
    return false;

  if (keyword.IsEmpty())
    return SetError();

  if (keyword == kTrailerKeyword)
    current_state_ = State::kCrossRefV4TrailerCheck;

  // Go to next item.
  current_offset_ = parser_->GetPos();
  return true;
}

bool CPDF_CrossRefAvail::CheckCrossRefV4Trailer() {
  parser_->SetPos(current_offset_);

  std::unique_ptr<CPDF_Dictionary> trailer =
      ToDictionary(parser_->GetObjectBody(nullptr));
  if (CheckReadProblems())
    return false;

  if (!trailer)
    return SetError();

  if (ToReference(trailer->GetObjectFor(kEncryptKey)))
    return SetError();

  const int32_t xrefpos =
      GetDirectInteger(trailer.get(), kPrevCrossRefFieldKey);
  if (xrefpos &&
      pdfium::base::IsValueInRangeForNumericType<FX_FILESIZE>(xrefpos))
    AddCrossRefForCheck(static_cast<FX_FILESIZE>(xrefpos));

  const int32_t stream_xref_offset =
      GetDirectInteger(trailer.get(), kPrevCrossRefStreamOffsetFieldKey);
  if (stream_xref_offset &&
      pdfium::base::IsValueInRangeForNumericType<FX_FILESIZE>(
          stream_xref_offset))
    AddCrossRefForCheck(static_cast<FX_FILESIZE>(stream_xref_offset));

  // Goto check next crossref
  current_state_ = State::kCrossRefCheck;
  return true;
}

bool CPDF_CrossRefAvail::CheckCrossRefStream() {
  auto cross_ref =
      parser_->GetIndirectObject(nullptr, CPDF_SyntaxParser::ParseType::kLoose);
  if (CheckReadProblems())
    return false;

  const CPDF_Dictionary* trailer =
      cross_ref && cross_ref->IsStream() ? cross_ref->GetDict() : nullptr;
  if (!trailer)
    return SetError();

  if (ToReference(trailer->GetObjectFor(kEncryptKey)))
    return SetError();

  CPDF_Name* type_name = ToName(trailer->GetObjectFor(kTypeFieldKey));
  if (type_name && type_name->GetString() == kXRefKeyword) {
    const int32_t xrefpos = trailer->GetIntegerFor(kPrevCrossRefFieldKey);
    if (xrefpos &&
        pdfium::base::IsValueInRangeForNumericType<FX_FILESIZE>(xrefpos))
      AddCrossRefForCheck(static_cast<FX_FILESIZE>(xrefpos));
  }
  // Goto check next crossref
  current_state_ = State::kCrossRefCheck;
  return true;
}

void CPDF_CrossRefAvail::AddCrossRefForCheck(FX_FILESIZE crossref_offset) {
  if (registered_crossrefs_.count(crossref_offset))
    return;

  cross_refs_for_check_.push(crossref_offset);
  registered_crossrefs_.insert(crossref_offset);
}

bool CPDF_CrossRefAvail::SetError() {
  current_status_ = CPDF_DataAvail::DataError;
  return false;
}

fxcrt::RetainPtr<CPDF_ReadValidator> CPDF_CrossRefAvail::GetValidator() {
  return parser_->GetValidator();
}
