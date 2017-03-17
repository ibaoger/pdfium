// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_widetextread.h"

#include <algorithm>

#include "core/fxcrt/fx_ext.h"
#include "xfa/fgas/crt/fgas_codepage.h"

CXFA_WideTextRead::CXFA_WideTextRead(const CFX_WideString& wsBuffer)
    : m_wsBuffer(wsBuffer), m_iPosition(0) {}

CXFA_WideTextRead::~CXFA_WideTextRead() {}

uint32_t CXFA_WideTextRead::GetAccessModes() const {
  return FX_STREAMACCESS_Read | FX_STREAMACCESS_Text;
}

int32_t CXFA_WideTextRead::GetLength() const {
  return m_wsBuffer.GetLength() * sizeof(wchar_t);
}

int32_t CXFA_WideTextRead::Seek(FX_STREAMSEEK eSeek, int32_t iOffset) {
  switch (eSeek) {
    case FX_STREAMSEEK_Begin:
      m_iPosition = iOffset;
      break;
    case FX_STREAMSEEK_Current:
      m_iPosition += iOffset;
      break;
    case FX_STREAMSEEK_End:
      m_iPosition = m_wsBuffer.GetLength() + iOffset;
      break;
  }
  m_iPosition = CFX_Clamp(0, m_iPosition, m_wsBuffer.GetLength());
  return GetPosition();
}

int32_t CXFA_WideTextRead::GetPosition() {
  return m_iPosition * sizeof(wchar_t);
}

bool CXFA_WideTextRead::IsEOF() const {
  return m_iPosition >= m_wsBuffer.GetLength();
}

int32_t CXFA_WideTextRead::ReadData(uint8_t* pBuffer, int32_t iBufferSize) {
  return 0;
}

int32_t CXFA_WideTextRead::ReadString(wchar_t* pStr,
                                      int32_t iMaxLength,
                                      bool& bEOS) {
  iMaxLength = std::min(iMaxLength, m_wsBuffer.GetLength() - m_iPosition);
  if (iMaxLength == 0)
    return 0;

  FXSYS_wcsncpy(pStr, m_wsBuffer.c_str() + m_iPosition, iMaxLength);
  m_iPosition += iMaxLength;
  bEOS = IsEOF();
  return iMaxLength;
}

int32_t CXFA_WideTextRead::WriteData(const uint8_t* pBuffer,
                                     int32_t iBufferSize) {
  return 0;
}

int32_t CXFA_WideTextRead::WriteString(const wchar_t* pStr, int32_t iLength) {
  return 0;
}

bool CXFA_WideTextRead::SetLength(int32_t iLength) {
  return false;
}

int32_t CXFA_WideTextRead::GetBOM(uint8_t bom[4]) const {
  return 0;
}

uint16_t CXFA_WideTextRead::GetCodePage() const {
  return (sizeof(wchar_t) == 2) ? FX_CODEPAGE_UTF16LE : FX_CODEPAGE_UTF32LE;
}

uint16_t CXFA_WideTextRead::SetCodePage(uint16_t wCodePage) {
  return GetCodePage();
}

CFX_RetainPtr<IFGAS_Stream> CXFA_WideTextRead::CreateSharedStream(
    uint32_t dwAccess,
    int32_t iOffset,
    int32_t iLength) {
  return nullptr;
}

CFX_WideString CXFA_WideTextRead::GetSrcText() const {
  return m_wsBuffer;
}
