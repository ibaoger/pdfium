// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_object_walker.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"

class CPDF_ObjectWalker::SubObjectsIterator {
 public:
  virtual ~SubObjectsIterator() {}

  bool IsStarted() const { return is_started_; }

  bool virtual IsFinished() const = 0;

  const CPDF_Object* Increment() {
    if (!IsStarted()) {
      Start();
      is_started_ = true;
    }
    while (!IsFinished()) {
      const CPDF_Object* result = IncrementImpl();
      if (result)
        return result;
    }
    return nullptr;
  }

  const CPDF_Object* object() const { return object_; }

  size_t current_index() const { return current_index_; }

  const CFX_ByteString& dict_key() const { return dict_key_; }

 protected:
  explicit SubObjectsIterator(const CPDF_Object* object) : object_(object) {
    ASSERT(object_);
  }

  virtual const CPDF_Object* IncrementImpl() = 0;
  virtual void Start() = 0;

  void set_dict_key(const CFX_ByteString& key) { dict_key_ = key; }
  void set_current_index(size_t current_index) {
    current_index_ = current_index;
  }

 private:
  const CPDF_Object* object_;
  bool is_started_ = false;
  size_t current_index_ = 0;
  CFX_ByteString dict_key_;
};

class CPDF_ObjectWalker::StreamIterator
    : public CPDF_ObjectWalker::SubObjectsIterator {
 public:
  explicit StreamIterator(const CPDF_Stream* stream)
      : SubObjectsIterator(stream) {}
  ~StreamIterator() override {}

  bool IsFinished() const override { return IsStarted() && !stream_dict_; }

  const CPDF_Object* IncrementImpl() override {
    ASSERT(IsStarted());
    ASSERT(!IsFinished());
    const CPDF_Object* result = stream_dict_;
    stream_dict_ = nullptr;
    return result;
  }

  void Start() override {
    ASSERT(!IsStarted());
    stream_dict_ = object()->GetDict();
  }

 private:
  const CPDF_Object* stream_dict_ = nullptr;
};

class CPDF_ObjectWalker::DictionaryIterator
    : public CPDF_ObjectWalker::SubObjectsIterator {
 public:
  explicit DictionaryIterator(const CPDF_Dictionary* dictionary)
      : SubObjectsIterator(dictionary) {}
  ~DictionaryIterator() override {}

  bool IsFinished() const override {
    return IsStarted() && dict_iterator_ == object()->GetDict()->end();
  }

  const CPDF_Object* IncrementImpl() override {
    ASSERT(IsStarted());
    ASSERT(!IsFinished());
    const CPDF_Object* result = dict_iterator_->second.get();
    set_dict_key(dict_iterator_->first);
    ++dict_iterator_;
    return result;
  }

  void Start() override {
    ASSERT(!IsStarted());
    dict_iterator_ = object()->GetDict()->begin();
  }

 private:
  CPDF_Dictionary::const_iterator dict_iterator_;
};

class CPDF_ObjectWalker::ArrayIterator
    : public CPDF_ObjectWalker::SubObjectsIterator {
 public:
  explicit ArrayIterator(const CPDF_Array* array) : SubObjectsIterator(array) {}

  ~ArrayIterator() override {}

  bool IsFinished() const override {
    return IsStarted() && arr_iterator_ == object()->AsArray()->end();
  }

  const CPDF_Object* IncrementImpl() override {
    ASSERT(IsStarted());
    ASSERT(!IsFinished());
    const CPDF_Object* result = arr_iterator_->get();
    set_current_index(arr_iterator_ - object()->AsArray()->begin());
    ++arr_iterator_;
    return result;
  }

  void Start() override { arr_iterator_ = object()->AsArray()->begin(); }

 public:
  CPDF_Array::const_iterator arr_iterator_;
};

// static
std::unique_ptr<CPDF_ObjectWalker::SubObjectsIterator>
CPDF_ObjectWalker::MakeIterator(const CPDF_Object* object) {
  if (object->IsStream())
    return pdfium::MakeUnique<StreamIterator>(object->AsStream());
  if (object->IsDictionary())
    return pdfium::MakeUnique<DictionaryIterator>(object->AsDictionary());
  if (object->IsArray())
    return pdfium::MakeUnique<ArrayIterator>(object->AsArray());
  return nullptr;
}

CPDF_ObjectWalker::CPDF_ObjectWalker(const CPDF_Object* root)
    : next_object_(root), parent_object_(nullptr), array_index_(0) {}

CPDF_ObjectWalker::~CPDF_ObjectWalker() {}

const CPDF_Object* CPDF_ObjectWalker::GetNext() {
  while (!stack_.empty() || next_object_) {
    if (next_object_) {
      auto new_iterator = MakeIterator(next_object_);
      if (new_iterator) {
        // Schedule walk within composite objects.
        stack_.push(std::move(new_iterator));
      }
      auto* result = next_object_;
      next_object_ = nullptr;
      return result;
    }

    SubObjectsIterator* it = stack_.top().get();
    if (it->IsFinished()) {
      stack_.pop();
    } else {
      next_object_ = it->Increment();
      parent_object_ = it->object();
      array_index_ = it->current_index();
      dict_key_ = it->dict_key();
    }
  }
  dict_key_ = CFX_ByteString();
  array_index_ = 0;
  return nullptr;
}

void CPDF_ObjectWalker::SkipWalkIntoCurrentObject() {
  if (stack_.empty() || stack_.top()->IsStarted())
    return;
  stack_.pop();
}
