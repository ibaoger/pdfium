// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FPDFAPI_PARSER_CPDF_OBJECT_WRITER_H_
#define CORE_FPDFAPI_PARSER_CPDF_OBJECT_WRITER_H_

#include <memory>

#include "core/fxcrt/cfx_unowned_ptr.h"

class IFX_ArchiveStream;
class CPDF_Object;
class CPDF_Stream;
class CFX_ByteString;

class CPDF_ObjectWriter {
 public:
  enum class WriteFlags : uint32_t {
    kNone = 0,
    // TODO(art-snake): Implement kEncode and kEncrypt
  };

  explicit CPDF_ObjectWriter(IFX_ArchiveStream* archive);

  ~CPDF_ObjectWriter();

  bool WriteObjectBody(const CPDF_Object* object,
                       WriteFlags flags = WriteFlags::kNone);
  bool WriteReference(uint32_t ref_objnum);
  bool WriteName(const CFX_ByteString& name);

 private:
  class Tail;
  bool WriteTail(const Tail* tail);
  bool WriteStream(const CPDF_Stream* stream);

  CFX_UnownedPtr<IFX_ArchiveStream> archive_;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_OBJECT_WRITER_H_
