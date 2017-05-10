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

namespace {

const int32_t kObjectStreamMaxSize = 200;

bool WriteTrailer(CPDF_Document* pDocument,
                  IFX_ArchiveStream* pFile,
                  CPDF_Array* pIDArray) {
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
      if (!pFile->WriteString(("/")) ||
          !pFile->WriteString(PDF_NameEncode(key).AsStringC())) {
        return false;
      }

      if (!pValue->IsInline()) {
        if (!pFile->WriteString(" ") ||
            !pFile->WriteDWord(pValue->GetObjNum()) ||
            !pFile->WriteString(" 0 R ")) {
          return false;
        }
      } else if (!pValue->WriteTo(pFile)) {
        return false;
      }
    }
    if (pIDArray) {
      if (!pFile->WriteString(("/ID")) || !pIDArray->WriteTo(pFile))
        return false;
    }
    return true;
  }
  if (!pFile->WriteString("\r\n/Root ") ||
      !pFile->WriteDWord(pDocument->GetRoot()->GetObjNum()) ||
      !pFile->WriteString(" 0 R\r\n")) {
    return false;
  }

  if (pDocument->GetInfo()) {
    if (!pFile->WriteString("/Info ") ||
        !pFile->WriteDWord(pDocument->GetInfo()->GetObjNum()) ||
        !pFile->WriteString(" 0 R\r\n")) {
      return false;
    }
  }
  if (pIDArray) {
    if (!pFile->WriteString(("/ID")) || !pIDArray->WriteTo(pFile))
      return -1;
  }
  return true;
}

bool WriteEncryptDictObjectReference(uint32_t dwObjNum,
                                     IFX_ArchiveStream* pFile) {
  ASSERT(pFile);

  if (!pFile->WriteString("/Encrypt") || !pFile->WriteString(" ") ||
      !pFile->WriteDWord(dwObjNum) || !pFile->WriteString(" 0 R ")) {
    return false;
  }
  return true;
}

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

bool CPDF_XRefStream::CompressIndirectObject(uint32_t dwObjNum,
                                             const CPDF_Object* pObj,
                                             CPDF_Creator* pCreator) {
  ASSERT(pCreator);

  m_ObjStream.CompressIndirectObject(dwObjNum, pObj);
  if (m_ObjStream.ItemCount() < kObjectStreamMaxSize &&
      m_ObjStream.IsNotFull()) {
    return true;
  }
  return EndObjectStream(pCreator, true);
}

bool CPDF_XRefStream::CompressIndirectObject(uint32_t dwObjNum,
                                             const uint8_t* pBuffer,
                                             uint32_t dwSize,
                                             CPDF_Creator* pCreator) {
  ASSERT(pCreator);

  m_ObjStream.CompressIndirectObject(dwObjNum, pBuffer, dwSize);
  if (m_ObjStream.ItemCount() < kObjectStreamMaxSize &&
      m_ObjStream.IsNotFull()) {
    return true;
  }
  return EndObjectStream(pCreator, true);
}

bool CPDF_XRefStream::EndObjectStream(CPDF_Creator* pCreator, bool bEOF) {
  FX_FILESIZE objOffset = 0;
  if (bEOF) {
    objOffset = m_ObjStream.End(pCreator);
    if (objOffset < 0)
      return false;
  }

  if (!m_ObjStream.GetObjectNumber())
    m_ObjStream.SetObjectNumber(pCreator->GetNextObjectNumber());

  int32_t iSize = m_ObjStream.ItemCount();
  size_t iSeg = m_IndexArray.size();
  if (!pCreator->IsIncremental()) {
    if (m_dwTempObjNum == 0) {
      AppendIndex0(m_Buffer, true);
      m_dwTempObjNum++;
    }
    uint32_t end_num = m_IndexArray.back().objnum + m_IndexArray.back().count;
    int index = 0;
    for (; m_dwTempObjNum < end_num; m_dwTempObjNum++) {
      if (pCreator->HasObjectNumber(m_dwTempObjNum)) {
        if (index >= iSize ||
            m_dwTempObjNum != m_ObjStream.GetObjectNumberForItem(index)) {
          AppendIndex1(m_Buffer, pCreator->GetObjectOffset(m_dwTempObjNum));
        } else {
          AppendIndex2(m_Buffer, m_ObjStream.GetObjectNumber(), index++);
        }
      } else {
        AppendIndex0(m_Buffer, false);
      }
    }
    if (iSize > 0 && bEOF)
      pCreator->SetObjectOffset(m_ObjStream.GetObjectNumber(), objOffset);

    m_iSeg = iSeg;
    if (bEOF)
      m_ObjStream.Start();

    return true;
  }

  for (auto it = m_IndexArray.begin() + m_iSeg; it != m_IndexArray.end();
       ++it) {
    for (uint32_t m = it->objnum; m < it->objnum + it->count; ++m) {
      if (m_ObjStream.GetIndex() >= iSize ||
          m != m_ObjStream.GetObjectNumberForItem(it - m_IndexArray.begin())) {
        AppendIndex1(m_Buffer, pCreator->GetObjectOffset(m));
      } else {
        AppendIndex2(m_Buffer, m_ObjStream.GetObjectNumber(),
                     m_ObjStream.GetIndex());
        m_ObjStream.IncrementIndex();
      }
    }
  }
  if (iSize > 0 && bEOF) {
    AppendIndex1(m_Buffer, objOffset);
    m_IndexArray.push_back({m_ObjStream.GetObjectNumber(), 1});
    iSeg += 1;
  }
  m_iSeg = iSeg;
  if (bEOF)
    m_ObjStream.Start();

  return true;
}

bool CPDF_XRefStream::GenerateXRefStream(CPDF_Creator* pCreator, bool bEOF) {
  uint32_t objnum = pCreator->GetNextObjectNumber();
  IFX_ArchiveStream* pFile = pCreator->GetFile();
  FX_FILESIZE offset_tmp = pFile->CurrentOffset();

  if (pCreator->IsIncremental()) {
    AddObjectNumberToIndexArray(objnum);
  } else {
    for (; m_dwTempObjNum < pCreator->GetLastObjectNumber(); m_dwTempObjNum++) {
      if (pCreator->HasObjectNumber(m_dwTempObjNum))
        AppendIndex1(m_Buffer, pCreator->GetObjectOffset(m_dwTempObjNum));
      else
        AppendIndex0(m_Buffer, false);
    }
  }

  AppendIndex1(m_Buffer, offset_tmp);

  if (!pFile->WriteDWord(objnum) ||
      !pFile->WriteString(" 0 obj\r\n<</Type /XRef/W[1 4 2]/Index[")) {
    return false;
  }

  if (!pCreator->IsIncremental()) {
    if (!pFile->WriteDWord(0) || !pFile->WriteString(" ") ||
        !pFile->WriteDWord(objnum + 1)) {
      return false;
    }
  } else {
    for (const auto& pair : m_IndexArray) {
      if (!pFile->WriteDWord(pair.objnum) || !pFile->WriteString(" ") ||
          !pFile->WriteDWord(pair.count) || !pFile->WriteString(" ")) {
        return false;
      }
    }
  }
  if (!pFile->WriteString("]/Size ") || !pFile->WriteDWord(objnum + 1))
    return false;

  if (m_PrevOffset > 0) {
    if (!pFile->WriteString("/Prev "))
      return false;

    char offset_buf[20];
    memset(offset_buf, 0, sizeof(offset_buf));
    FXSYS_i64toa(m_PrevOffset, offset_buf, 10);
    int32_t offset_len = (int32_t)FXSYS_strlen(offset_buf);
    if (!pFile->WriteBlock(offset_buf, offset_len))
      return false;
  }

  CPDF_FlateEncoder encoder(m_Buffer.GetBuffer(), m_Buffer.GetLength(), true,
                            true);
  if (!pFile->WriteString("/Filter /FlateDecode") ||
      !pFile->WriteString("/DecodeParms<</Columns 7/Predictor 12>>") ||
      !pFile->WriteString("/Length ") ||
      !pFile->WriteDWord(encoder.GetSize())) {
    return false;
  }

  if (bEOF) {
    if (!WriteTrailer(pCreator->GetDocument(), pFile, pCreator->GetIDArray()))
      return false;

    if (CPDF_Dictionary* encryptDict = pCreator->GetEncryptDict()) {
      uint32_t dwEncryptObjNum = encryptDict->GetObjNum();
      if (dwEncryptObjNum == 0)
        dwEncryptObjNum = pCreator->GetEncryptObjectNumber();

      if (!WriteEncryptDictObjectReference(dwEncryptObjNum, pFile))
        return false;
    }
  }

  if (!pFile->WriteString(">>stream\r\n") ||
      !pFile->WriteBlock(encoder.GetData(), encoder.GetSize()) ||
      !pFile->WriteString("\r\nendstream\r\nendobj\r\n")) {
    return false;
  }

  m_PrevOffset = offset_tmp;
  return true;
}

bool CPDF_XRefStream::End(CPDF_Creator* pCreator, bool bEOF) {
  if (!EndObjectStream(pCreator, bEOF))
    return false;
  return GenerateXRefStream(pCreator, bEOF);
}

bool CPDF_XRefStream::EndXRefStream(CPDF_Creator* pCreator) {
  if (!pCreator->IsIncremental()) {
    AppendIndex0(m_Buffer, true);
    for (uint32_t i = 1; i < pCreator->GetLastObjectNumber() + 1; i++) {
      if (pCreator->HasObjectNumber(i))
        AppendIndex1(m_Buffer, pCreator->GetObjectOffset(i));
      else
        AppendIndex0(m_Buffer, false);
    }
  } else {
    for (const auto& pair : m_IndexArray) {
      for (uint32_t j = pair.objnum; j < pair.objnum + pair.count; ++j)
        AppendIndex1(m_Buffer, pCreator->GetObjectOffset(j));
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
