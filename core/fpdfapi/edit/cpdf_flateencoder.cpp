// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/edit/cpdf_flateencoder.h"

#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"

namespace {

constexpr char kFilterKey[] = "Filter";
constexpr char kDecodeParmsKey[] = "DecodeParms";
constexpr char kLengthKey[] = "Length";

}  // namespace

CPDF_FlateEncoder::CPDF_FlateEncoder(const CPDF_Stream* pStream,
                                     bool bFlateEncode)
    : m_dwSize(0), m_pAcc(pdfium::MakeRetain<CPDF_StreamAcc>(pStream)) {
  m_pAcc->LoadAllData(true);

  bool bHasFilter = pStream && pStream->HasFilter();
  if (bHasFilter && !bFlateEncode) {
    auto pDestAcc = pdfium::MakeRetain<CPDF_StreamAcc>(pStream);
    pDestAcc->LoadAllData();

    m_dwSize = pDestAcc->GetSize();
    m_pData = pDestAcc->DetachData();

    m_ChangedValues.push_back(std::make_pair(kFilterKey, nullptr));
    m_ChangedValues.push_back(std::make_pair(kDecodeParmsKey, nullptr));
    m_ChangedValues.push_back(
        std::make_pair(kLengthKey, pdfium::MakeUnique<CPDF_Number>(m_dwSize)));
    return;
  }
  if (bHasFilter || !bFlateEncode) {
    m_pData = const_cast<uint8_t*>(m_pAcc->GetData());
    m_dwSize = m_pAcc->GetSize();
    ASSERT(m_dwSize == pStream->GetRawSize());
    ASSERT(static_cast<int>(m_dwSize) ==
           pStream->GetDict()->GetIntegerFor(kLengthKey));
    return;
  }

  // TODO(thestig): Move to Init() and check return value.
  uint8_t* buffer = nullptr;
  ::FlateEncode(m_pAcc->GetData(), m_pAcc->GetSize(), &buffer, &m_dwSize);

  m_pData = std::unique_ptr<uint8_t, FxFreeDeleter>(buffer);

  m_ChangedValues.push_back(std::make_pair(
      kFilterKey, pdfium::MakeUnique<CPDF_Name>(nullptr, "FlateDecode")));
  m_ChangedValues.push_back(
      std::make_pair(kLengthKey, pdfium::MakeUnique<CPDF_Number>(m_dwSize)));
  m_ChangedValues.push_back(std::make_pair(kDecodeParmsKey, nullptr));
}

CPDF_FlateEncoder::~CPDF_FlateEncoder() {}
