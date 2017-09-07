// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FPDFAPI_PARSER_CPDF_OBJECT_WRITER_H_
#define CORE_FPDFAPI_PARSER_CPDF_OBJECT_WRITER_H_

#include <memory>

#include "core/fxcrt/cfx_unowned_ptr.h"

class CFX_ByteString;
class CPDF_CryptoHandler;
class CPDF_Object;
class CPDF_Stream;
class CPDF_String;
class IFX_ArchiveStream;

class CPDF_ObjectWriter {
 public:
  enum WriteFlags {
    kNone = 0,
    kDecode = 1 << 0,
    kEncode = 1 << 1,
    kEncrypt = 1 << 2,
  };

  explicit CPDF_ObjectWriter(IFX_ArchiveStream* archive);

  CPDF_ObjectWriter(IFX_ArchiveStream* archive,
                    CPDF_CryptoHandler* crypto_handler);

  ~CPDF_ObjectWriter();

  bool WriteObjectBody(const CPDF_Object* object,
                       int flags = WriteFlags::kNone);

  bool WriteIndirectObject(const CPDF_Object* object,
                           uint32_t objnum,
                           uint32_t gennum,
                           int flags = WriteFlags::kNone);

  bool WriteIndirectObject(const CPDF_Object* object,
                           int flags = WriteFlags::kNone);

  bool WriteReference(uint32_t ref_objnum);
  bool WriteName(const CFX_ByteString& name);

 private:
  class Encryptor;
  class StreamWriter;
  class Tail;

  bool WriteTail(const Tail* tail);
  bool WriteObjectBodyInternal(const CPDF_Object* object,
                               Encryptor* encryptor,
                               int flags);
  bool WriteString(const CPDF_String* obj_str, Encryptor* encryptor);

  CFX_UnownedPtr<IFX_ArchiveStream> archive_;
  CFX_UnownedPtr<CPDF_CryptoHandler> crypto_handler_;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_OBJECT_WRITER_H_
