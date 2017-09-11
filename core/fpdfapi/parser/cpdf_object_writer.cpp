// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_object_writer.h"

#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "core/fpdfapi/parser/cpdf_crypto_handler.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_object_walker.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fxcrt/cfx_maybe_owned.h"
#include "core/fxcrt/fx_stream.h"

namespace {

class ScopedDictionaryChanger {
 public:
  explicit ScopedDictionaryChanger(CPDF_Dictionary* dict) : dict_(dict) {}

  ~ScopedDictionaryChanger() {
    for (auto& it : changed_values_) {
      dict_->SetFor(it.first, std::move(it.second));
    }
  }

  void SetFor(const CFX_ByteString& key, std::unique_ptr<CPDF_Object> obj) {
    changed_values_.insert(std::make_pair(key, dict_->RemoveFor(key)));
    dict_->SetFor(key, std::move(obj));
  }

  void RemoveFor(const CFX_ByteString& key) {
    changed_values_.insert(std::make_pair(key, dict_->RemoveFor(key)));
    dict_->RemoveFor(key);
  }

 private:
  CPDF_Dictionary* dict_;
  std::map<const CFX_ByteString, std::unique_ptr<CPDF_Object>> changed_values_;
};
}  // namespace

class CPDF_ObjectWriter::Encryptor {
 public:
  Encryptor(CPDF_CryptoHandler* handler, uint32_t objnum, uint32_t gennum)
      : handler_(handler), objnum_(objnum), gennum_(gennum) {
    ASSERT(handler);
  }
  ~Encryptor() {}

  std::pair<std::unique_ptr<uint8_t, FxFreeDeleter>, uint32_t> Encrypt(
      const uint8_t* data,
      uint32_t data_size) {
    uint32_t encrypted_size =
        handler_->EncryptGetSize(objnum_, gennum_, data, data_size);
    std::unique_ptr<uint8_t, FxFreeDeleter> encrypted_data(
        FX_Alloc(uint8_t, encrypted_size));
    handler_->EncryptContent(objnum_, gennum_, data, data_size,
                             encrypted_data.get(), encrypted_size);
    return std::make_pair(std::move(encrypted_data), encrypted_size);
  }

 private:
  CFX_UnownedPtr<CPDF_CryptoHandler> handler_;
  const uint32_t objnum_;
  const uint32_t gennum_;
};

class CPDF_ObjectWriter::StreamWriter {
 public:
  StreamWriter(const CPDF_Stream* stream, Encryptor* encryptor, int flags)
      : stream_(stream), scoped_dictionary_changer_(stream->GetDict()) {
    LoadStream(flags & WriteFlags::kDecode);
    if (flags & WriteFlags::kEncode)
      EncodeStream();
    if (flags & WriteFlags::kEncrypt)
      EncryptStream(encryptor);
  }

  ~StreamWriter() {}

  bool WriteStream(IFX_ArchiveStream* archive) const {
    return archive->WriteString("stream\r\n") &&
           archive->WriteBlock(data_.Get(), data_size_) &&
           archive->WriteString("\r\nendstream");
  }

 private:
  void LoadStream(bool decode) {
    auto stream_access = (pdfium::MakeRetain<CPDF_StreamAcc>(stream_));
    stream_access->LoadAllData(!decode);
    data_size_ = stream_access->GetSize();
    data_ = stream_access->DetachData();
    if (decode)
      scoped_dictionary_changer_.RemoveFor("Filter");
  }

  void EncodeStream() {
    if (stream_->HasFilter()) {
      // Stream already encoded.
      return;
    }
    uint8_t* buffer = nullptr;
    uint32_t encoded_size = 0;
    if (!::FlateEncode(data_.Get(), data_size_, &buffer, &encoded_size))
      return;

    data_ = std::unique_ptr<uint8_t, FxFreeDeleter>(buffer);
    data_size_ = encoded_size;
    scoped_dictionary_changer_.SetFor(
        "Length",
        pdfium::MakeUnique<CPDF_Number>(static_cast<int>(encoded_size)));
    scoped_dictionary_changer_.SetFor(
        "Filter", pdfium::MakeUnique<CPDF_Name>(nullptr, "FlateDecode"));
    scoped_dictionary_changer_.RemoveFor("DecodeParms");
  }

  void EncryptStream(Encryptor* encryptor) {
    if (!encryptor)
      return;

    std::tie(data_, data_size_) = encryptor->Encrypt(data_.Get(), data_size_);
    scoped_dictionary_changer_.SetFor(
        "Length",
        pdfium::MakeUnique<CPDF_Number>(static_cast<int>(data_size_)));
  }

  const CPDF_Stream* stream_;
  uint32_t data_size_;
  CFX_MaybeOwned<uint8_t, FxFreeDeleter> data_;
  CFX_RetainPtr<CPDF_StreamAcc> stream_access_;
  ScopedDictionaryChanger scoped_dictionary_changer_;
};

class CPDF_ObjectWriter::Tail {
 public:
  Tail(const CFX_ByteStringC& string,
       std::unique_ptr<StreamWriter> stream_writer)
      : string_(string), stream_writer_(std::move(stream_writer)) {}

  const CFX_ByteStringC& string() const { return string_; }
  const StreamWriter* stream_writer() const { return stream_writer_.get(); }

 private:
  const CFX_ByteStringC string_;
  std::unique_ptr<StreamWriter> stream_writer_;
};

CPDF_ObjectWriter::CPDF_ObjectWriter(IFX_ArchiveStream* archive)
    : CPDF_ObjectWriter(archive, nullptr) {}

CPDF_ObjectWriter::CPDF_ObjectWriter(IFX_ArchiveStream* archive,
                                     CPDF_CryptoHandler* crypto_handler)
    : archive_(archive), crypto_handler_(crypto_handler) {
  ASSERT(archive_);
}

CPDF_ObjectWriter::~CPDF_ObjectWriter() {}

bool CPDF_ObjectWriter::WriteTail(const Tail* tail) {
  ASSERT(tail);
  if (!tail->string().IsEmpty() && !archive_->WriteString(tail->string()))
    return false;

  if (tail->stream_writer() &&
      !tail->stream_writer()->WriteStream(archive_.Get()))
    return false;

  return true;
}

bool CPDF_ObjectWriter::WriteObjectBody(const CPDF_Object* object, int flags) {
  if (flags & WriteFlags::kEncrypt)
    return false;

  return WriteObjectBodyInternal(object, nullptr, flags);
}

bool CPDF_ObjectWriter::WriteIndirectObject(const CPDF_Object* object,
                                            uint32_t objnum,
                                            uint32_t gennum,
                                            int flags) {
  std::unique_ptr<Encryptor> encryptor;
  if ((flags & WriteFlags::kEncrypt) && crypto_handler_)
    encryptor =
        pdfium::MakeUnique<Encryptor>(crypto_handler_.Get(), objnum, gennum);

  return object && archive_->WriteDWord(objnum) && archive_->WriteString(" ") &&
         archive_->WriteDWord(gennum) && archive_->WriteString(" obj\r\n") &&
         WriteObjectBodyInternal(object, encryptor.get(), flags) &&
         archive_->WriteString("\r\nendobj\r\n");
}

bool CPDF_ObjectWriter::WriteIndirectObject(const CPDF_Object* object,
                                            int flags) {
  return object && !object->IsInline() &&
         WriteIndirectObject(object, object->GetObjNum(), object->GetGenNum(),
                             flags);
}

bool CPDF_ObjectWriter::WriteObjectBodyInternal(const CPDF_Object* object,
                                                Encryptor* encryptor,
                                                int flags) {
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

    if (encryptor && (flags & WriteFlags::kEncrypt) &&
        ToDictionary(walker.GetParent()) &&
        walker.GetParent()->GetDict()->IsSignatureDict() &&
        walker.dictionary_key() == "Contents") {
      if (!WriteObjectBodyInternal(obj, nullptr, flags & ~WriteFlags::kEncrypt))
        return false;
      walker.SkipWalkIntoCurrentObject();
      continue;
    }

    if (obj->IsArray()) {
      if (!archive_->WriteString("["))
        return false;

      tails.push_back(pdfium::MakeUnique<Tail>("]", nullptr));
    } else if (obj->IsDictionary()) {
      if (!archive_->WriteString("<<"))
        return false;

      tails.push_back(pdfium::MakeUnique<Tail>(">>", nullptr));
    } else if (obj->IsStream()) {
      tails.push_back(pdfium::MakeUnique<Tail>(
          nullptr,
          pdfium::MakeUnique<StreamWriter>(obj->AsStream(), encryptor, flags)));
    } else if (obj->IsString()) {
      if (!WriteString(obj->AsString(),
                       (flags & WriteFlags::kEncrypt) ? encryptor : nullptr))
        return false;

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

bool CPDF_ObjectWriter::WriteReference(uint32_t ref_objnum) {
  return archive_->WriteString(" ") && archive_->WriteDWord(ref_objnum) &&
         archive_->WriteString(" 0 R ");
}

bool CPDF_ObjectWriter::WriteName(const CFX_ByteString& name) {
  return archive_->WriteString("/") &&
         archive_->WriteString(PDF_NameEncode(name).AsStringC());
}

bool CPDF_ObjectWriter::WriteString(const CPDF_String* obj_str,
                                    Encryptor* encryptor) {
  CFX_ByteString str = obj_str->GetString();
  const uint8_t* data = reinterpret_cast<const uint8_t*>(str.c_str());
  uint32_t data_size = str.GetLength();
  std::unique_ptr<uint8_t, FxFreeDeleter> encrypted_data;
  if (encryptor) {
    std::tie(encrypted_data, data_size) = encryptor->Encrypt(data, data_size);
    data = encrypted_data.get();
  }
  return archive_->WriteString(
      PDF_EncodeString(CFX_ByteString(data, data_size), obj_str->IsHex())
          .AsStringC());
}
