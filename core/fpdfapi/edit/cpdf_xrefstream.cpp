// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/edit/cpdf_xrefstream.h"

#include "core/fpdfapi/edit/cpdf_creator.h"
#include "core/fpdfapi/edit/cpdf_flateencoder.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"

#define PDF_OBJECTSTREAM_MAXLENGTH (256 * 1024)

namespace {

void AppendIndex0(CFX_ByteTextBuf& buffer, bool bFirstObject) {
  buffer.AppendByte(0);
  buffer.AppendByte(0);
  buffer.AppendByte(0);
  buffer.AppendByte(0);
  buffer.AppendByte(0);

  const uint8_t byte = bFirstObject ? 0xFF : 0;
  buffer.AppendByte(byte);
  buffer.AppendByte(byte);
}

void AppendIndex1(CFX_ByteTextBuf& buffer, FX_FILESIZE offset) {
  buffer.AppendByte(1);
  buffer.AppendByte(static_cast<uint8_t>(offset >> 24));
  buffer.AppendByte(static_cast<uint8_t>(offset >> 16));
  buffer.AppendByte(static_cast<uint8_t>(offset >> 8));
  buffer.AppendByte(static_cast<uint8_t>(offset));
  buffer.AppendByte(0);
  buffer.AppendByte(0);
}

void AppendIndex2(CFX_ByteTextBuf& buffer, uint32_t objnum, int32_t index) {
  buffer.AppendByte(2);
  buffer.AppendByte(static_cast<uint8_t>(objnum >> 24));
  buffer.AppendByte(static_cast<uint8_t>(objnum >> 16));
  buffer.AppendByte(static_cast<uint8_t>(objnum >> 8));
  buffer.AppendByte(static_cast<uint8_t>(objnum));
  buffer.AppendByte(static_cast<uint8_t>(index >> 8));
  buffer.AppendByte(static_cast<uint8_t>(index));
}

int32_t WriteTrailer(CPDF_Creator* pCreator, CFX_FileBufferArchive* pFile) {
  CPDF_Document* pDocument = pCreator->GetDocument();
  CPDF_Array* pIDArray = pCreator->GetIDArray();
  FX_FILESIZE offset = 0;
  int32_t len = 0;
  CPDF_Parser* pParser = pDocument->GetParser();
  if (pParser) {
    CPDF_Dictionary* p = pParser->GetTrailer();
    for (const auto& it : *p) {
      const CFX_ByteString& key = it.first;
      CPDF_Object* pValue = it.second.get();
      if (key == "Encrypt" || key == "Size" || key == "Filter" ||
          key == "Index" || key == "Length" || key == "Prev" || key == "W" ||
          key == "XRefStm" || key == "Type" || key == "ID") {
        continue;
      }
      if (key == "DecodeParms")
        continue;
      if (pFile->AppendString(("/")) < 0)
        return -1;
      if ((len = pFile->AppendString(PDF_NameEncode(key).AsStringC())) < 0)
        return -1;

      offset += len + 1;
      if (!pValue->IsInline()) {
        if (pFile->AppendString(" ") < 0)
          return -1;
        if ((len = pFile->AppendDWord(pValue->GetObjNum())) < 0)
          return -1;
        if (pFile->AppendString(" 0 R ") < 0)
          return -1;

        offset += len + 6;
      } else {
        if (pCreator->AppendObject(pValue, pFile, offset) < 0)
          return -1;
      }
    }
    if (pIDArray) {
      if (pFile->AppendString(("/ID")) < 0)
        return -1;

      offset += 3;
      if (pCreator->AppendObject(pIDArray, pFile, offset) < 0)
        return -1;
    }
    return offset;
  }
  if (pFile->AppendString("\r\n/Root ") < 0)
    return -1;
  if ((len = pFile->AppendDWord(pDocument->GetRoot()->GetObjNum())) < 0)
    return -1;
  if (pFile->AppendString(" 0 R\r\n") < 0)
    return -1;

  offset += len + 14;
  if (pDocument->GetInfo()) {
    if (pFile->AppendString("/Info ") < 0)
      return -1;
    if ((len = pFile->AppendDWord(pDocument->GetInfo()->GetObjNum())) < 0)
      return -1;
    if (pFile->AppendString(" 0 R\r\n") < 0)
      return -1;

    offset += len + 12;
  }
  if (pIDArray) {
    if (pFile->AppendString(("/ID")) < 0)
      return -1;

    offset += 3;
    if (pCreator->AppendObject(pIDArray, pFile, offset) < 0)
      return -1;
  }
  return offset;
}

int32_t WriteEncrypt(const CPDF_Dictionary* pEncryptDict,
                     uint32_t dwObjNum,
                     CFX_FileBufferArchive* pFile) {
  if (!pEncryptDict)
    return 0;

  ASSERT(pFile);
  FX_FILESIZE offset = 0;
  int32_t len = 0;
  if (pFile->AppendString("/Encrypt") < 0)
    return -1;

  offset += 8;
  if (pFile->AppendString(" ") < 0)
    return -1;
  if ((len = pFile->AppendDWord(dwObjNum)) < 0)
    return -1;
  if (pFile->AppendString(" 0 R ") < 0)
    return -1;

  offset += len + 6;
  return offset;
}

}  // namespace

CPDF_XRefStream::CPDF_XRefStream()
    : m_PrevOffset(0), m_dwTempObjNum(0), m_iSeg(0) {}

CPDF_XRefStream::~CPDF_XRefStream() {}

bool CPDF_XRefStream::Start() {
  m_IndexArray.clear();
  m_Buffer.Clear();
  m_iSeg = 0;
  return true;
}

int32_t CPDF_XRefStream::CompressIndirectObject(uint32_t dwObjNum,
                                                const CPDF_Object* pObj,
                                                CPDF_Creator* pCreator) {
  if (!pCreator)
    return 0;

  m_ObjStream.CompressIndirectObject(dwObjNum, pObj);
  if (pdfium::CollectionSize<int32_t>(m_ObjStream.m_Items) <
          pCreator->GetObjectStreamSize() &&
      m_ObjStream.m_Buffer.GetLength() < PDF_OBJECTSTREAM_MAXLENGTH) {
    return 1;
  }
  return EndObjectStream(pCreator);
}

int32_t CPDF_XRefStream::CompressIndirectObject(uint32_t dwObjNum,
                                                const uint8_t* pBuffer,
                                                uint32_t dwSize,
                                                CPDF_Creator* pCreator) {
  if (!pCreator)
    return 0;

  m_ObjStream.CompressIndirectObject(dwObjNum, pBuffer, dwSize);
  if (pdfium::CollectionSize<int32_t>(m_ObjStream.m_Items) <
          pCreator->GetObjectStreamSize() &&
      m_ObjStream.m_Buffer.GetLength() < PDF_OBJECTSTREAM_MAXLENGTH) {
    return 1;
  }
  return EndObjectStream(pCreator);
}

int32_t CPDF_XRefStream::EndObjectStream(CPDF_Creator* pCreator, bool bEOF) {
  FX_FILESIZE objOffset = 0;
  if (bEOF) {
    objOffset = m_ObjStream.End(pCreator);
    if (objOffset < 0)
      return -1;
  }

  uint32_t& dwObjStmNum = m_ObjStream.m_dwObjNum;
  if (!dwObjStmNum)
    dwObjStmNum = pCreator->GetNextObjectNumber();

  int32_t iSize = pdfium::CollectionSize<int32_t>(m_ObjStream.m_Items);
  size_t iSeg = m_IndexArray.size();
  auto* objectOffsets = pCreator->GetObjectOffsets();
  if (!pCreator->IsIncremental()) {
    if (m_dwTempObjNum == 0) {
      AppendIndex0(m_Buffer, true);
      m_dwTempObjNum++;
    }
    uint32_t end_num = m_IndexArray.back().objnum + m_IndexArray.back().count;
    int index = 0;

    for (; m_dwTempObjNum < end_num; m_dwTempObjNum++) {
      auto it = objectOffsets->find(m_dwTempObjNum);
      if (it != objectOffsets->end()) {
        if (index >= iSize ||
            m_dwTempObjNum != m_ObjStream.m_Items[index].objnum) {
          AppendIndex1(m_Buffer, it->second);
        } else {
          AppendIndex2(m_Buffer, dwObjStmNum, index++);
        }
      } else {
        AppendIndex0(m_Buffer, false);
      }
    }
    if (iSize > 0 && bEOF)
      (*objectOffsets)[dwObjStmNum] = objOffset;

    m_iSeg = iSeg;
    if (bEOF)
      m_ObjStream.Start();

    return 1;
  }
  for (auto it = m_IndexArray.begin() + m_iSeg; it != m_IndexArray.end();
       ++it) {
    for (uint32_t m = it->objnum; m < it->objnum + it->count; ++m) {
      if (m_ObjStream.m_index >= iSize ||
          m != m_ObjStream.m_Items[it - m_IndexArray.begin()].objnum) {
        AppendIndex1(m_Buffer, (*objectOffsets)[m]);
      } else {
        AppendIndex2(m_Buffer, dwObjStmNum, m_ObjStream.m_index++);
      }
    }
  }
  if (iSize > 0 && bEOF) {
    AppendIndex1(m_Buffer, objOffset);
    m_IndexArray.push_back({dwObjStmNum, 1});
    iSeg += 1;
  }
  m_iSeg = iSeg;
  if (bEOF)
    m_ObjStream.Start();

  return 1;
}

bool CPDF_XRefStream::GenerateXRefStream(CPDF_Creator* pCreator, bool bEOF) {
  FX_FILESIZE offset_tmp = pCreator->GetOffset();
  uint32_t objnum = pCreator->GetNextObjectNumber();
  CFX_FileBufferArchive* pFile = pCreator->GetFile();
  if (pCreator->IsIncremental()) {
    AddObjectNumberToIndexArray(objnum);
  } else {
    auto* objectOffsets = pCreator->GetObjectOffsets();
    for (; m_dwTempObjNum < pCreator->GetLastObjectNumber(); m_dwTempObjNum++) {
      auto it = objectOffsets->find(m_dwTempObjNum);
      if (it != objectOffsets->end())
        AppendIndex1(m_Buffer, it->second);
      else
        AppendIndex0(m_Buffer, false);
    }
  }

  AppendIndex1(m_Buffer, offset_tmp);

  int32_t len = pFile->AppendDWord(objnum);
  if (len < 0)
    return false;

  pCreator->IncrementOffset(len);
  if ((len = pFile->AppendString(" 0 obj\r\n<</Type /XRef/W[1 4 2]/Index[")) <
      0) {
    return false;
  }
  pCreator->IncrementOffset(len);
  if (!pCreator->IsIncremental()) {
    if ((len = pFile->AppendDWord(0)) < 0)
      return false;
    if ((len = pFile->AppendString(" ")) < 0)
      return false;

    pCreator->IncrementOffset(len + 1);
    if ((len = pFile->AppendDWord(objnum + 1)) < 0)
      return false;

    pCreator->IncrementOffset(len);
  } else {
    for (const auto& pair : m_IndexArray) {
      if ((len = pFile->AppendDWord(pair.objnum)) < 0)
        return false;
      if (pFile->AppendString(" ") < 0)
        return false;

      pCreator->IncrementOffset(len + 1);
      if ((len = pFile->AppendDWord(pair.count)) < 0)
        return false;
      if (pFile->AppendString(" ") < 0)
        return false;

      pCreator->IncrementOffset(len + 1);
    }
  }
  if (pFile->AppendString("]/Size ") < 0)
    return false;
  if ((len = pFile->AppendDWord(objnum + 1)) < 0)
    return false;

  pCreator->IncrementOffset(len + 7);
  if (m_PrevOffset > 0) {
    if (pFile->AppendString("/Prev ") < 0)
      return false;

    char offset_buf[20];
    memset(offset_buf, 0, sizeof(offset_buf));
    FXSYS_i64toa(m_PrevOffset, offset_buf, 10);
    int32_t offset_len = (int32_t)FXSYS_strlen(offset_buf);
    if (pFile->AppendBlock(offset_buf, offset_len) < 0)
      return false;

    pCreator->IncrementOffset(offset_len + 6);
  }

  CPDF_FlateEncoder encoder(m_Buffer.GetBuffer(), m_Buffer.GetLength(), true,
                            true);
  if (pFile->AppendString("/Filter /FlateDecode") < 0)
    return false;

  pCreator->IncrementOffset(20);
  if ((len = pFile->AppendString("/DecodeParms<</Columns 7/Predictor 12>>")) <
      0) {
    return false;
  }

  pCreator->IncrementOffset(len);
  if (pFile->AppendString("/Length ") < 0)
    return false;
  if ((len = pFile->AppendDWord(encoder.m_dwSize)) < 0)
    return false;

  pCreator->IncrementOffset(len + 8);
  if (bEOF) {
    if ((len = WriteTrailer(pCreator, pFile)) < 0) {
      return false;
    }
    pCreator->IncrementOffset(len);
    if (pCreator->GetEncryptDict()) {
      uint32_t dwEncryptObjNum = pCreator->GetEncryptDict()->GetObjNum();
      if (dwEncryptObjNum == 0)
        dwEncryptObjNum = pCreator->GetEncryptObjectNumber();
      if ((len = WriteEncrypt(pCreator->GetEncryptDict(), dwEncryptObjNum,
                              pFile)) < 0) {
        return false;
      }
      pCreator->IncrementOffset(len);
    }
  }
  if ((len = pFile->AppendString(">>stream\r\n")) < 0)
    return false;

  pCreator->IncrementOffset(len);
  if (pFile->AppendBlock(encoder.m_pData.Get(), encoder.m_dwSize) < 0)
    return false;
  if ((len = pFile->AppendString("\r\nendstream\r\nendobj\r\n")) < 0)
    return false;

  pCreator->IncrementOffset(encoder.m_dwSize + len);
  m_PrevOffset = offset_tmp;
  return true;
}

bool CPDF_XRefStream::End(CPDF_Creator* pCreator, bool bEOF) {
  if (EndObjectStream(pCreator, bEOF) < 0)
    return false;
  return GenerateXRefStream(pCreator, bEOF);
}

bool CPDF_XRefStream::EndXRefStream(CPDF_Creator* pCreator) {
  auto* objectOffsets = pCreator->GetObjectOffsets();
  if (!pCreator->IsIncremental()) {
    AppendIndex0(m_Buffer, true);

    for (uint32_t i = 1; i < pCreator->GetLastObjectNumber() + 1; i++) {
      auto it = objectOffsets->find(i);
      if (it != objectOffsets->end())
        AppendIndex1(m_Buffer, it->second);
      else
        AppendIndex0(m_Buffer, false);
    }
  } else {
    for (const auto& pair : m_IndexArray) {
      for (uint32_t j = pair.objnum; j < pair.objnum + pair.count; ++j)
        AppendIndex1(m_Buffer, (*objectOffsets)[j]);
    }
  }
  return GenerateXRefStream(pCreator, false);
}

void CPDF_XRefStream::AddObjectNumberToIndexArray(uint32_t objnum) {
  if (m_IndexArray.empty()) {
    m_IndexArray.push_back({objnum, 1});
    return;
  }

  uint32_t next_objnum = m_IndexArray.back().objnum + m_IndexArray.back().count;
  if (objnum == next_objnum)
    m_IndexArray.back().count += 1;
  else
    m_IndexArray.push_back({objnum, 1});
}
