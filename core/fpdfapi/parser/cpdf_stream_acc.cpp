// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_stream_acc.h"

#include "core/fpdfapi/parser/fpdf_parser_decode.h"

CPDF_StreamAcc::CPDF_StreamAcc(const CPDF_Stream* pStream)
    : m_pStream(pStream) {}

CPDF_StreamAcc::~CPDF_StreamAcc() {
  if (m_bNewBuf)
    FX_Free(m_pData);
}

void CPDF_StreamAcc::LoadAllData(bool bRawAccess,
                                 uint32_t estimated_size,
                                 bool bImageAcc) {
  if (!m_pStream)
    return;

  bool bProcessRawData = bRawAccess || !m_pStream->HasFilter();
  if (bProcessRawData && m_pStream->IsMemoryBased()) {
    m_dwSize = m_pStream->GetRawSize();
    m_pData = m_pStream->GetRawData();
    return;
  }
  uint32_t dwSrcSize = m_pStream->GetRawSize();
  if (dwSrcSize == 0)
    return;

  uint8_t* pSrcData;
  if (m_pStream->IsMemoryBased()) {
    pSrcData = m_pStream->GetRawData();
  } else {
    pSrcData = FX_Alloc(uint8_t, dwSrcSize);
    if (!m_pStream->ReadRawData(0, pSrcData, dwSrcSize)) {
      FX_Free(pSrcData);
      return;
    }
    if (bProcessRawData) {
      m_pData = pSrcData;
      m_dwSize = dwSrcSize;
      m_bNewBuf = true;
      return;
    }
  }
  uint8_t* pDecodedData = nullptr;
  uint32_t dwDecodedSize = 0;
  if (PDF_DataDecode(pSrcData, dwSrcSize, m_pStream->GetDict(), estimated_size,
                     bImageAcc, &pDecodedData, &dwDecodedSize, &m_ImageDecoder,
                     &m_pImageParam)) {
    m_pData = pDecodedData;
    m_dwSize = dwDecodedSize;
    if (!m_pStream->IsMemoryBased())
      FX_Free(pSrcData);
    m_bNewBuf = true;
    return;
  }

  m_pData = pSrcData;
  m_dwSize = dwSrcSize;
  m_bNewBuf = !m_pStream->IsMemoryBased();
}

void CPDF_StreamAcc::LoadAllDataFiltered() {
  LoadAllData(false, 0, false);
}

void CPDF_StreamAcc::LoadAllDataRaw() {
  LoadAllData(true, 0, false);
}

CPDF_Dictionary* CPDF_StreamAcc::GetDict() const {
  return m_pStream ? m_pStream->GetDict() : nullptr;
}

const uint8_t* CPDF_StreamAcc::GetData() const {
  return GetDataHelper();
}

uint8_t* CPDF_StreamAcc::GetData() {
  return GetDataHelper();
}

uint32_t CPDF_StreamAcc::GetSize() const {
  if (m_bNewBuf)
    return m_dwSize;
  return m_pStream ? m_pStream->GetRawSize() : 0;
}

std::unique_ptr<uint8_t, FxFreeDeleter> CPDF_StreamAcc::DetachData() {
  if (m_bNewBuf) {
    std::unique_ptr<uint8_t, FxFreeDeleter> p(m_pData);
    m_pData = nullptr;
    m_dwSize = 0;
    return p;
  }
  std::unique_ptr<uint8_t, FxFreeDeleter> p(FX_Alloc(uint8_t, m_dwSize));
  memcpy(p.get(), m_pData, m_dwSize);
  return p;
}

uint8_t* CPDF_StreamAcc::GetDataHelper() const {
  if (m_bNewBuf)
    return m_pData;
  return m_pStream ? m_pStream->GetRawData() : nullptr;
}
