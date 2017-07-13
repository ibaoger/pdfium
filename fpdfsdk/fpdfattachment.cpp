// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_attachment.h"

#include "core/fpdfapi/page/cpdf_streamparser.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfdoc/cpdf_filespec.h"
#include "core/fpdfdoc/cpdf_nametree.h"
#include "fpdfsdk/fsdk_define.h"

namespace {

CPDF_Object* GetEmbeddedFileObject(FPDF_DOCUMENT document, int index) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc || index < 0)
    return nullptr;

  CPDF_NameTree nameTree(pDoc, "EmbeddedFiles");
  if (static_cast<size_t>(index) >= nameTree.GetCount())
    return nullptr;

  CFX_ByteString csName;
  return nameTree.LookupValueAndName(index, &csName);
}

unsigned long Utf16EncodeMaybeCopyAndReturnLength(const CFX_WideString& src,
                                                  void* buffer,
                                                  unsigned long buflen) {
  CFX_ByteString text = src.UTF16LE_Encode();
  unsigned long len = text.GetLength();
  if (buffer && buflen >= len)
    memcpy(buffer, text.c_str(), len);

  return len;
}

}  // namespace

DLLEXPORT int STDCALL FPDFDoc_GetAttachmentCount(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 0;

  return CPDF_NameTree(pDoc, "EmbeddedFiles").GetCount();
}

DLLEXPORT unsigned long STDCALL
FPDFDoc_GetAttachmentName(FPDF_DOCUMENT document,
                          int index,
                          void* buffer,
                          unsigned long buflen) {
  CPDF_Object* pFile = GetEmbeddedFileObject(document, index);
  if (!pFile)
    return 0;

  return Utf16EncodeMaybeCopyAndReturnLength(CPDF_FileSpec(pFile).GetFileName(),
                                             buffer, buflen);
}

DLLEXPORT unsigned long STDCALL
FPDFDoc_GetAttachmentDate(FPDF_DOCUMENT document,
                          int index,
                          FPDFATTACHMENT_DATETYPE type,
                          void* buffer,
                          unsigned long buflen) {
  CPDF_Object* pFile = GetEmbeddedFileObject(document, index);
  if (!pFile)
    return 0;

  CPDF_Dictionary* pParamsDict = CPDF_FileSpec(pFile).GetParamsDict();
  if (!pParamsDict)
    return 0;

  CFX_WideString value = pParamsDict->GetUnicodeTextFor(
      type == FPDFATTACHMENT_DATETYPE_Creation ? "CreationDate" : "ModDate");
  return Utf16EncodeMaybeCopyAndReturnLength(value, buffer, buflen);
}

DLLEXPORT unsigned long STDCALL
FPDFDoc_GetAttachmentCheckSum(FPDF_DOCUMENT document,
                              int index,
                              void* buffer,
                              unsigned long buflen) {
  CPDF_Object* pFile = GetEmbeddedFileObject(document, index);
  if (!pFile)
    return 0;

  CPDF_FileSpec filespec(pFile);
  CPDF_Dictionary* pParamsDict = filespec.GetParamsDict();
  if (!pParamsDict)
    return 0;

  CFX_ByteString key = "CheckSum";
  CFX_WideString value =
      pParamsDict->GetObjectFor(key)->AsString()->IsHex()
          ? CPDF_String(nullptr,
                        PDF_EncodeString(pParamsDict->GetStringFor(key), true),
                        false)
                .GetUnicodeText()
          : pParamsDict->GetUnicodeTextFor(key);
  return Utf16EncodeMaybeCopyAndReturnLength(value, buffer, buflen);
}

DLLEXPORT unsigned long STDCALL
FPDFDoc_GetAttachmentFile(FPDF_DOCUMENT document,
                          int index,
                          void* buffer,
                          unsigned long buflen) {
  CPDF_Object* pFile = GetEmbeddedFileObject(document, index);
  if (!pFile)
    return 0;

  CPDF_Stream* pFileStream = CPDF_FileSpec(pFile).GetFileStream();
  if (!pFileStream)
    return 0;

  uint8_t* data = pFileStream->GetRawData();
  uint32_t len = pFileStream->GetRawSize();
  CPDF_Dictionary* pFileDict = pFileStream->GetDict();
  if (!pFileDict || pFileDict->GetStringFor("Filter").IsEmpty()) {
    if (buffer && buflen >= len)
      memcpy(buffer, data, len);

    return len;
  }

  // Decode the stream if a stream filter is specified.
  uint8_t* decodedData = nullptr;
  uint32_t decodedLen = 0;
  CPDF_StreamParser::DecodeInlineStream(
      data, len, pFileDict->GetIntegerFor("Width"),
      pFileDict->GetIntegerFor("Height"), pFileDict->GetStringFor("Filter"),
      pFileDict->GetDictFor("DecodeParms"), &decodedData, &decodedLen);
  if (buffer && buflen >= decodedLen)
    memcpy(buffer, decodedData, decodedLen);

  FX_Free(decodedData);
  return decodedLen;
}
