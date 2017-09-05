// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_object_writer.h"

#include <string>
#include <vector>

#include "core/fpdfapi/parser/cpdf_object_walker.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fxcrt/fx_stream.h"

class CPDF_ObjectWriter::Tail {
 public:
  Tail(const CFX_ByteStringC& string, const CPDF_Stream* stream)
      : string_(string), stream_(stream) {}

  const CFX_ByteStringC& string() const { return string_; }
  const CPDF_Stream* stream() const { return stream_; }

 private:
  const CFX_ByteStringC string_;
  const CPDF_Stream* stream_;
};

CPDF_ObjectWriter::CPDF_ObjectWriter(IFX_ArchiveStream* archive)
    : archive_(archive) {
  ASSERT(archive);
}

CPDF_ObjectWriter::~CPDF_ObjectWriter() {}

bool CPDF_ObjectWriter::WriteTail(const Tail* tail) {
  ASSERT(tail);
  if (!tail->string().IsEmpty() && !archive_->WriteString(tail->string()))
    return false;

  if (tail->stream() && !WriteStream(tail->stream()))
    return false;

  return true;
}

bool CPDF_ObjectWriter::WriteObjectBody(const CPDF_Object* object,
                                        WriteFlags flags) {
  if (!object)
    return false;

  std::vector<std::unique_ptr<Tail>> tails;
  CPDF_ObjectWalker walker(object);
  while (const CPDF_Object* obj = walker.GetNext()) {
    while (tails.size() > walker.current_depth()) {
      if (!WriteTail(tails.back().get()))
        return false;
      tails.pop_back();
    }

    // TODO(art-snake): Do we needs this inlining check?
    if (walker.GetParent() && !obj->IsInline()) {
      if (!WriteReference(obj->GetObjNum()))
        return false;
      walker.SkipWalkIntoCurrentObject();
      continue;
    }

    if (ToDictionary(walker.GetParent()) && !WriteName(walker.dictionary_key()))
      return false;

    if (obj->IsArray()) {
      if (!archive_->WriteString("["))
        return false;

      tails.push_back(pdfium::MakeUnique<Tail>("]", nullptr));
    } else if (obj->IsDictionary()) {
      if (!archive_->WriteString("<<"))
        return false;

      tails.push_back(pdfium::MakeUnique<Tail>(">>", nullptr));
    } else if (obj->IsStream()) {
      tails.push_back(pdfium::MakeUnique<Tail>(nullptr, obj->AsStream()));
    } else if (!obj->WriteTo(archive_.Get())) {
      return false;
    }
  }

  for (auto it = tails.rbegin(); it != tails.rend(); ++it) {
    if (!WriteTail(it->get()))
      return false;
  }
  return true;
}

bool CPDF_ObjectWriter::WriteStream(const CPDF_Stream* stream) {
  ASSERT(stream);
  if (!archive_->WriteString("stream\r\n"))
    return false;

  auto pAcc = pdfium::MakeRetain<CPDF_StreamAcc>(stream);
  pAcc->LoadAllData(true);
  return archive_->WriteBlock(pAcc->GetData(), pAcc->GetSize()) &&
         archive_->WriteString("\r\nendstream");
}

bool CPDF_ObjectWriter::WriteReference(uint32_t ref_objnum) {
  return archive_->WriteString(" ") && archive_->WriteDWord(ref_objnum) &&
         archive_->WriteString(" 0 R ");
}

bool CPDF_ObjectWriter::WriteName(const CFX_ByteString& name) {
  return archive_->WriteString("/") &&
         archive_->WriteString(PDF_NameEncode(name).AsStringC());
}
