// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_object_walker.h"

#include "core/fpdfapi/parser/cpdf_array.h"

class CPDF_ObjectWalker::SubObjectsIterator {
 public:
  explicit SubObjectsIterator(CPDF_Object* object) : object_(object) {
    ASSERT(object_);
  }
  ~SubObjectsIterator() {}

  bool IsStarted() const { return is_started_; }

  bool IsFinished() const {
    if (!IsStarted())
      return false;
    if (object_->IsDictionary()) {
      return dict_iterator_ == object_->GetDict()->end();
    }
    if (object_->IsArray()) {
      return array_pos_ >= object_->AsArray()->GetCount();
    }
    if (object_->IsStream()) {
      return !stream_dict_;
    }
    return true;
  }

  CPDF_Object* Increment() {
    if (!IsStarted())
      SetStarted();
    while (!IsFinished()) {
      CPDF_Object* result = IncrementImpl();
      if (result)
        return result;
    }
    return nullptr;
  }

 private:
  CPDF_Object* IncrementImpl() {
    ASSERT(IsStarted());
    ASSERT(!IsFinished());
    if (object_->IsDictionary()) {
      CPDF_Object* result = dict_iterator_->second.get();
      ++dict_iterator_;
      return result;
    }
    if (object_->IsArray()) {
      CPDF_Object* result = object_->AsArray()->GetObjectAt(array_pos_);
      ++array_pos_;
      return result;
    }
    if (object_->IsStream()) {
      CPDF_Object* result = stream_dict_;
      stream_dict_ = nullptr;
      return result;
    }
    return nullptr;
  }

  void SetStarted() {
    if (IsStarted())
      return;
    is_started_ = true;
    if (object_->IsDictionary()) {
      dict_iterator_ = object_->GetDict()->begin();
    }
    if (object_->IsStream()) {
      stream_dict_ = object_->GetDict();
    }
  }

  CPDF_Object* object_;
  CPDF_Object* stream_dict_ = nullptr;
  size_t array_pos_ = 0;
  CPDF_Dictionary::const_iterator dict_iterator_;
  bool is_started_ = false;
};

CPDF_ObjectWalker::CPDF_ObjectWalker(CPDF_Object* root) : next_object_(root) {}

CPDF_ObjectWalker::~CPDF_ObjectWalker() {}

CPDF_Object* CPDF_ObjectWalker::GetNext() {
  while (!stack_.empty() || next_object_) {
    if (next_object_) {
      if (next_object_->IsDictionary() || next_object_->IsArray() ||
          next_object_->IsStream()) {
        // Schedule walk within composite objects.
        stack_.push(pdfium::MakeUnique<SubObjectsIterator>(next_object_));
      }
      auto* result = next_object_;
      next_object_ = nullptr;
      ++count_my;
      return result;
    }

    SubObjectsIterator* it = stack_.top().get();
    if (it->IsFinished()) {
      stack_.pop();
    } else {
      next_object_ = it->Increment();
    }
  }
  return nullptr;
}
