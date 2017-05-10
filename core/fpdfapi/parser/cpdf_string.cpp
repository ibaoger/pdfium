// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_string.h"

#include <utility>

#include "core/fpdfapi/edit/cpdf_encryptor.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "third_party/base/ptr_util.h"

CPDF_String::CPDF_String() : m_bHex(false) {}

CPDF_String::CPDF_String(CFX_WeakPtr<CFX_ByteStringPool> pPool,
                         const CFX_ByteString& str,
                         bool bHex)
    : m_String(str), m_bHex(bHex) {
  if (pPool)
    m_String = pPool->Intern(m_String);
}

CPDF_String::CPDF_String(CFX_WeakPtr<CFX_ByteStringPool> pPool,
                         const CFX_WideString& str)
    : m_String(PDF_EncodeText(str)), m_bHex(false) {
  if (pPool)
    m_String = pPool->Intern(m_String);
}

CPDF_String::~CPDF_String() {}

CPDF_Object::Type CPDF_String::GetType() const {
  return STRING;
}

std::unique_ptr<CPDF_Object> CPDF_String::Clone() const {
  auto pRet = pdfium::MakeUnique<CPDF_String>();
  pRet->m_String = m_String;
  pRet->m_bHex = m_bHex;
  return std::move(pRet);
}

CFX_ByteString CPDF_String::GetString() const {
  return m_String;
}

void CPDF_String::SetString(const CFX_ByteString& str) {
  m_String = str;
}

bool CPDF_String::IsString() const {
  return true;
}

CPDF_String* CPDF_String::AsString() {
  return this;
}

const CPDF_String* CPDF_String::AsString() const {
  return this;
}

CFX_WideString CPDF_String::GetUnicodeText() const {
  return PDF_DecodeText(m_String);
}

bool CPDF_String::WriteTo(IFX_ArchiveStream* archive) const {
  return archive->WriteString(
      PDF_EncodeString(GetString(), IsHex()).AsStringC());
}

bool CPDF_String::WriteDirectTo(IFX_ArchiveStream* archive,
                                uint32_t objnum,
                                bool encrypt,
                                CPDF_CryptoHandler* crypto_handler) const {
  if (!encrypt || !crypto_handler)
    return WriteTo(archive);

  CFX_ByteString str = GetString();
  CPDF_Encryptor encryptor(
      crypto_handler, objnum,
      reinterpret_cast<uint8_t*>(const_cast<char*>(str.c_str())),
      str.GetLength());

  return archive->WriteString(
      PDF_EncodeString(CFX_ByteString(encryptor.GetData(), encryptor.GetSize()),
                       IsHex())
          .AsStringC());
}
