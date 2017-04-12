// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/crt/ifgas_stream.h"

#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || \
    _FX_OS_ == _FX_WIN64_
#include <io.h>
#endif

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_ext.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fgas/crt/fgas_codepage.h"

namespace {

class IFGAS_StreamImp {
 public:
  virtual ~IFGAS_StreamImp() {}

  virtual FX_FILESIZE GetLength() const = 0;
  virtual FX_FILESIZE GetPosition() = 0;
  virtual void Seek(FX_STREAMSEEK eSeek, FX_FILESIZE iOffset) = 0;
  virtual bool IsEOF() const = 0;
  virtual FX_FILESIZE ReadData(uint8_t* pBuffer, FX_FILESIZE iBufferSize) = 0;
  virtual FX_FILESIZE ReadString(wchar_t* pStr,
                                 FX_FILESIZE iMaxLength,
                                 bool* bEOS) = 0;
  virtual void WriteData(const uint8_t* pBuffer, FX_FILESIZE iBufferSize) = 0;
  virtual void WriteString(const wchar_t* pStr, FX_FILESIZE iLength) = 0;

 protected:
  IFGAS_StreamImp();
};

class CFGAS_FileReadStreamImp : public IFGAS_StreamImp {
 public:
  explicit CFGAS_FileReadStreamImp(
      const CFX_RetainPtr<IFX_SeekableReadStream>& pFileRead);
  ~CFGAS_FileReadStreamImp() override {}

  // IFGAS_StreamImp:
  FX_FILESIZE GetLength() const override;
  FX_FILESIZE GetPosition() override { return m_iPosition; }
  void Seek(FX_STREAMSEEK eSeek, FX_FILESIZE iOffset) override;
  bool IsEOF() const override;
  FX_FILESIZE ReadData(uint8_t* pBuffer, FX_FILESIZE iBufferSize) override;
  FX_FILESIZE ReadString(wchar_t* pStr,
                         FX_FILESIZE iMaxLength,
                         bool* bEOS) override;
  void WriteData(const uint8_t* pBuffer, FX_FILESIZE iBufferSize) override {}
  void WriteString(const wchar_t* pStr, FX_FILESIZE iLength) override {}

 private:
  CFX_RetainPtr<IFX_SeekableReadStream> m_pFileRead;
  FX_FILESIZE m_iPosition;
};

class CFGAS_FileWriteStreamImp : public IFGAS_StreamImp {
 public:
  CFGAS_FileWriteStreamImp(
      const CFX_RetainPtr<IFX_SeekableWriteStream>& pFileWrite);
  ~CFGAS_FileWriteStreamImp() override {}

  // IFGAS_StreamImp:
  FX_FILESIZE GetLength() const override;
  FX_FILESIZE GetPosition() override { return m_iPosition; }
  void Seek(FX_STREAMSEEK eSeek, FX_FILESIZE iOffset) override;
  bool IsEOF() const override;
  FX_FILESIZE ReadData(uint8_t* pBuffer, FX_FILESIZE iBufferSize) override {
    return 0;
  }
  FX_FILESIZE ReadString(wchar_t* pStr,
                         FX_FILESIZE iMaxLength,
                         bool* bEOS) override {
    return 0;
  }
  void WriteData(const uint8_t* pBuffer, FX_FILESIZE iBufferSize) override;
  void WriteString(const wchar_t* pStr, FX_FILESIZE iLength) override;

 private:
  CFX_RetainPtr<IFX_SeekableWriteStream> m_pFileWrite;
  FX_FILESIZE m_iPosition;
};

class CFGAS_TextStream : public IFGAS_Stream {
 public:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // IFGAS_Stream
  FX_FILESIZE GetLength() const override;
  FX_FILESIZE GetPosition() override;
  uint8_t GetBOMLength() const override;
  void Seek(FX_STREAMSEEK eSeek, FX_FILESIZE iOffset) override;
  bool IsEOF() const override;
  FX_FILESIZE ReadData(uint8_t* pBuffer, FX_FILESIZE iBufferSize) override;
  FX_FILESIZE ReadString(wchar_t* pStr,
                         FX_FILESIZE iMaxLength,
                         bool* bEOS) override;
  void WriteData(const uint8_t* pBuffer, FX_FILESIZE iBufferSize) override;
  void WriteString(const wchar_t* pStr, FX_FILESIZE iLength) override;
  uint16_t GetCodePage() const override;
  void SetCodePage(uint16_t wCodePage) override;

 private:
  CFGAS_TextStream(std::unique_ptr<IFGAS_StreamImp> imp, bool isWriteSteam);
  ~CFGAS_TextStream() override;

  void InitStream();

  uint16_t m_wCodePage;
  FX_FILESIZE m_wBOMLength;
  bool m_IsWriteStream;
  std::unique_ptr<IFGAS_StreamImp> m_pStreamImp;
};

class CFGAS_WideStringReadStream : public IFGAS_Stream {
 public:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // IFGAS_Stream
  FX_FILESIZE GetLength() const override;
  FX_FILESIZE GetPosition() override;
  uint8_t GetBOMLength() const override { return 0; }
  void Seek(FX_STREAMSEEK eSeek, FX_FILESIZE iOffset) override;
  bool IsEOF() const override;
  FX_FILESIZE ReadData(uint8_t* pBuffer, FX_FILESIZE iBufferSize) override {
    return 0;
  }
  FX_FILESIZE ReadString(wchar_t* pStr,
                         FX_FILESIZE iMaxLength,
                         bool* bEOS) override;
  void WriteData(const uint8_t* pBuffer, FX_FILESIZE iBufferSize) override {}
  void WriteString(const wchar_t* pStr, FX_FILESIZE iLength) override {}
  uint16_t GetCodePage() const override;
  void SetCodePage(uint16_t wCodePage) override {}

 private:
  explicit CFGAS_WideStringReadStream(const CFX_WideString& wsBuffer);
  ~CFGAS_WideStringReadStream() override;

  CFX_WideString m_wsBuffer;
  FX_FILESIZE m_iPosition;
};

IFGAS_StreamImp::IFGAS_StreamImp() {}

CFGAS_FileReadStreamImp::CFGAS_FileReadStreamImp(
    const CFX_RetainPtr<IFX_SeekableReadStream>& pFileRead)
    : m_pFileRead(pFileRead), m_iPosition(0) {
  ASSERT(m_pFileRead);
}

FX_FILESIZE CFGAS_FileReadStreamImp::GetLength() const {
  return m_pFileRead->GetSize();
}

void CFGAS_FileReadStreamImp::Seek(FX_STREAMSEEK eSeek, FX_FILESIZE iOffset) {
  switch (eSeek) {
    case FX_STREAMSEEK_Begin:
      m_iPosition = iOffset;
      break;
    case FX_STREAMSEEK_Current:
      m_iPosition += iOffset;
      break;
  }
  m_iPosition = pdfium::clamp(m_iPosition, 0L, m_pFileRead->GetSize());
}

bool CFGAS_FileReadStreamImp::IsEOF() const {
  return m_iPosition >= m_pFileRead->GetSize();
}

FX_FILESIZE CFGAS_FileReadStreamImp::ReadData(uint8_t* pBuffer,
                                              FX_FILESIZE iBufferSize) {
  ASSERT(pBuffer && iBufferSize > 0);

  iBufferSize = std::min(iBufferSize, m_pFileRead->GetSize() - m_iPosition);
  if (m_pFileRead->ReadBlock(pBuffer, m_iPosition, iBufferSize)) {
    pdfium::base::CheckedNumeric<FX_FILESIZE> new_pos = m_iPosition;
    new_pos += iBufferSize;
    if (!new_pos.IsValid())
      return 0;

    m_iPosition = new_pos.ValueOrDie();
    return iBufferSize;
  }
  return 0;
}

FX_FILESIZE CFGAS_FileReadStreamImp::ReadString(wchar_t* pStr,
                                                FX_FILESIZE iMaxLength,
                                                bool* bEOS) {
  ASSERT(pStr && iMaxLength > 0);

  iMaxLength = ReadData(reinterpret_cast<uint8_t*>(pStr), iMaxLength * 2) / 2;
  if (iMaxLength <= 0)
    return 0;

  FX_FILESIZE i = 0;
  while (i < iMaxLength && pStr[i] != L'\0')
    ++i;

  *bEOS = m_iPosition >= m_pFileRead->GetSize() || pStr[i] == L'\0';
  return i;
}

CFGAS_FileWriteStreamImp::CFGAS_FileWriteStreamImp(
    const CFX_RetainPtr<IFX_SeekableWriteStream>& pFileWrite)
    : m_pFileWrite(pFileWrite), m_iPosition(m_pFileWrite->GetSize()) {
  ASSERT(m_pFileWrite);
}

FX_FILESIZE CFGAS_FileWriteStreamImp::GetLength() const {
  return m_pFileWrite->GetSize();
}

void CFGAS_FileWriteStreamImp::Seek(FX_STREAMSEEK eSeek, FX_FILESIZE iOffset) {
  FX_FILESIZE iLength = GetLength();
  switch (eSeek) {
    case FX_STREAMSEEK_Begin:
      m_iPosition = iOffset;
      break;
    case FX_STREAMSEEK_Current:
      m_iPosition += iOffset;
      break;
  }
  m_iPosition = pdfium::clamp(m_iPosition, 0L, iLength);
}

bool CFGAS_FileWriteStreamImp::IsEOF() const {
  return m_iPosition >= GetLength();
}

void CFGAS_FileWriteStreamImp::WriteData(const uint8_t* pBuffer,
                                         FX_FILESIZE iBufferSize) {
  if (m_pFileWrite->WriteBlock(pBuffer, m_iPosition, iBufferSize)) {
    pdfium::base::CheckedNumeric<FX_FILESIZE> new_pos = m_iPosition;
    new_pos += iBufferSize;
    // TODO(dsinclair): Not sure what to do if we over flow ....
    if (!new_pos.IsValid())
      return;

    m_iPosition = new_pos.ValueOrDie();
  }
}

void CFGAS_FileWriteStreamImp::WriteString(const wchar_t* pStr,
                                           FX_FILESIZE iLength) {
  WriteData(reinterpret_cast<const uint8_t*>(pStr), iLength * sizeof(wchar_t));
}

CFGAS_TextStream::CFGAS_TextStream(std::unique_ptr<IFGAS_StreamImp> imp,
                                   bool isWriteStream)
    : m_wCodePage(FX_CODEPAGE_DefANSI),
      m_wBOMLength(0),
      m_IsWriteStream(isWriteStream),
      m_pStreamImp(std::move(imp)) {
  ASSERT(m_pStreamImp);
  InitStream();
}

CFGAS_TextStream::~CFGAS_TextStream() {}

#if _FX_ENDIAN_ == _FX_LITTLE_ENDIAN_
#define BOM_MASK 0x00FFFFFF
#define BOM_UTF8 0x00BFBBEF
#define BOM_UTF16_MASK 0x0000FFFF
#define BOM_UTF16_BE 0x0000FFFE
#define BOM_UTF16_LE 0x0000FEFF
#else
#define BOM_MASK 0xFFFFFF00
#define BOM_UTF8 0xEFBBBF00
#define BOM_UTF16_MASK 0xFFFF0000
#define BOM_UTF16_BE 0xFEFF0000
#define BOM_UTF16_LE 0xFFFE0000
#endif  // _FX_ENDIAN_ == _FX_LITTLE_ENDIAN_

void CFGAS_TextStream::InitStream() {
  FX_FILESIZE iPosition = m_pStreamImp->GetPosition();
  m_pStreamImp->Seek(FX_STREAMSEEK_Begin, 0);

  uint32_t bom;
  m_pStreamImp->ReadData(reinterpret_cast<uint8_t*>(&bom), 3);

  bom &= BOM_MASK;
  if (bom == BOM_UTF8) {
    m_wBOMLength = 3;
    m_wCodePage = FX_CODEPAGE_UTF8;
  } else {
    bom &= BOM_UTF16_MASK;
    if (bom == BOM_UTF16_BE) {
      m_wBOMLength = 2;
      m_wCodePage = FX_CODEPAGE_UTF16BE;
    } else if (bom == BOM_UTF16_LE) {
      m_wBOMLength = 2;
      m_wCodePage = FX_CODEPAGE_UTF16LE;
    } else {
      m_wBOMLength = 0;
      m_wCodePage = FXSYS_GetACP();
    }
  }

  m_pStreamImp->Seek(FX_STREAMSEEK_Begin, std::max(m_wBOMLength, iPosition));
}

FX_FILESIZE CFGAS_TextStream::GetLength() const {
  return m_pStreamImp->GetLength();
}

void CFGAS_TextStream::Seek(FX_STREAMSEEK eSeek, FX_FILESIZE iOffset) {
  m_pStreamImp->Seek(eSeek, iOffset);
}

FX_FILESIZE CFGAS_TextStream::GetPosition() {
  return m_pStreamImp->GetPosition();
}

bool CFGAS_TextStream::IsEOF() const {
  return m_pStreamImp->IsEOF();
}

FX_FILESIZE CFGAS_TextStream::ReadData(uint8_t* pBuffer,
                                       FX_FILESIZE iBufferSize) {
  ASSERT(pBuffer && iBufferSize > 0);

  if (m_IsWriteStream)
    return -1;

  FX_FILESIZE iLen = std::min(
      m_pStreamImp->GetLength() - m_pStreamImp->GetPosition(), iBufferSize);
  if (iLen <= 0)
    return 0;

  return m_pStreamImp->ReadData(pBuffer, iLen);
}

void CFGAS_TextStream::WriteData(const uint8_t* pBuffer,
                                 FX_FILESIZE iBufferSize) {
  ASSERT(pBuffer && iBufferSize > 0);

  if (!m_IsWriteStream)
    return;
  m_pStreamImp->WriteData(pBuffer, iBufferSize);
}

uint8_t CFGAS_TextStream::GetBOMLength() const {
  if (m_wBOMLength < 1)
    return 0;
  return m_wBOMLength;
}

uint16_t CFGAS_TextStream::GetCodePage() const {
  return m_wCodePage;
}

void CFGAS_TextStream::SetCodePage(uint16_t wCodePage) {
  if (m_wBOMLength > 0)
    return;

  m_wCodePage = wCodePage;
}

FX_FILESIZE CFGAS_TextStream::ReadString(wchar_t* pStr,
                                         FX_FILESIZE iMaxLength,
                                         bool* bEOS) {
  ASSERT(pStr && iMaxLength > 0);
  if (m_IsWriteStream)
    return -1;

  if (m_wCodePage == FX_CODEPAGE_UTF16LE ||
      m_wCodePage == FX_CODEPAGE_UTF16BE) {
    FX_FILESIZE iBytes = iMaxLength * 2;
    FX_FILESIZE iLen = m_pStreamImp->ReadData((uint8_t*)pStr, iBytes);
    iMaxLength = iLen / 2;
    if (sizeof(wchar_t) > 2)
      FX_UTF16ToWChar(pStr, iMaxLength);

#if _FX_ENDIAN_ == _FX_BIG_ENDIAN_
    if (m_wCodePage == FX_CODEPAGE_UTF16LE)
      FX_SwapByteOrder(pStr, iMaxLength);
#else
    if (m_wCodePage == FX_CODEPAGE_UTF16BE)
      FX_SwapByteOrder(pStr, iMaxLength);
#endif

  } else {
    FX_FILESIZE pos = m_pStreamImp->GetPosition();
    FX_FILESIZE iBytes = std::min(iMaxLength, m_pStreamImp->GetLength() - pos);
    if (iBytes > 0) {
      std::vector<uint8_t> buf(iBytes);

      FX_FILESIZE iLen = m_pStreamImp->ReadData(buf.data(), iBytes);
      int32_t iSrc = iLen;
      int32_t val;
      int32_t iDecode = FX_DecodeString(
          m_wCodePage, reinterpret_cast<const char*>(buf.data()), &iSrc, pStr,
          &val, true);
      iMaxLength = val;
      m_pStreamImp->Seek(FX_STREAMSEEK_Current, iSrc - iLen);
      if (iDecode < 1)
        return -1;
    } else {
      iMaxLength = 0;
    }
  }
  *bEOS = m_pStreamImp->IsEOF();
  return iMaxLength;
}

void CFGAS_TextStream::WriteString(const wchar_t* pStr, FX_FILESIZE iLength) {
  ASSERT(pStr && iLength > 0);
  if (!m_IsWriteStream)
    return;

  if (m_wCodePage != FX_CODEPAGE_UTF8)
    return;

  FX_FILESIZE len = iLength;
  CFX_UTF8Encoder encoder;
  while (len-- > 0)
    encoder.Input(*pStr++);

  CFX_ByteStringC bsResult = encoder.GetResult();
  m_pStreamImp->WriteData(reinterpret_cast<const uint8_t*>(bsResult.c_str()),
                          bsResult.GetLength());
}

CFGAS_WideStringReadStream::CFGAS_WideStringReadStream(
    const CFX_WideString& wsBuffer)
    : m_wsBuffer(wsBuffer), m_iPosition(0) {}

CFGAS_WideStringReadStream::~CFGAS_WideStringReadStream() {}

FX_FILESIZE CFGAS_WideStringReadStream::GetLength() const {
  return m_wsBuffer.GetLength() * sizeof(wchar_t);
}

void CFGAS_WideStringReadStream::Seek(FX_STREAMSEEK eSeek,
                                      FX_FILESIZE iOffset) {
  switch (eSeek) {
    case FX_STREAMSEEK_Begin:
      m_iPosition = iOffset;
      break;
    case FX_STREAMSEEK_Current:
      m_iPosition += iOffset;
      break;
  }
  m_iPosition = pdfium::clamp(m_iPosition, 0L,
                              static_cast<FX_FILESIZE>(m_wsBuffer.GetLength()));
}

FX_FILESIZE CFGAS_WideStringReadStream::GetPosition() {
  return m_iPosition * sizeof(wchar_t);
}

bool CFGAS_WideStringReadStream::IsEOF() const {
  return m_iPosition >= m_wsBuffer.GetLength();
}

FX_FILESIZE CFGAS_WideStringReadStream::ReadString(wchar_t* pStr,
                                                   FX_FILESIZE iMaxLength,
                                                   bool* bEOS) {
  iMaxLength = std::min(iMaxLength, m_wsBuffer.GetLength() - m_iPosition);
  if (iMaxLength == 0)
    return 0;

  FXSYS_wcsncpy(pStr, m_wsBuffer.c_str() + m_iPosition, iMaxLength);
  m_iPosition += iMaxLength;
  *bEOS = IsEOF();
  return iMaxLength;
}

uint16_t CFGAS_WideStringReadStream::GetCodePage() const {
  return (sizeof(wchar_t) == 2) ? FX_CODEPAGE_UTF16LE : FX_CODEPAGE_UTF32LE;
}

}  // namespace

// static
CFX_RetainPtr<IFGAS_Stream> IFGAS_Stream::CreateReadStream(
    const CFX_RetainPtr<IFX_SeekableReadStream>& pFileRead) {
  if (!pFileRead)
    return nullptr;

  return pdfium::MakeRetain<CFGAS_TextStream>(
      pdfium::MakeUnique<CFGAS_FileReadStreamImp>(pFileRead), false);
}

// static
CFX_RetainPtr<IFGAS_Stream> IFGAS_Stream::CreateWriteStream(
    const CFX_RetainPtr<IFX_SeekableWriteStream>& pFileWrite) {
  if (!pFileWrite)
    return nullptr;

  return pdfium::MakeRetain<CFGAS_TextStream>(
      pdfium::MakeUnique<CFGAS_FileWriteStreamImp>(pFileWrite), true);
}

// static
CFX_RetainPtr<IFGAS_Stream> IFGAS_Stream::CreateWideStringReadStream(
    const CFX_WideString& buffer) {
  return pdfium::MakeRetain<CFGAS_WideStringReadStream>(buffer);
}
