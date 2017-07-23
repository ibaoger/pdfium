// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_object_walker.h"

#include "core/fpdfapi/parser/cpdf_array.h"

CPDF_ObjectWalker::CPDF_ObjectWalker(CPDF_Object* root) : next_object_(root) {}

CPDF_Object* CPDF_ObjectWalker::GetNext() {
  while (!stack_.empty() || next_object_) {
    if (next_object_) {
      if (next_object_->IsDictionary() || next_object_->IsArray() ||
          next_object_->IsStream()) {
        stack_.push(Position(
            {next_object_, 0, CPDF_Dictionary::const_iterator(), false}));
      }
      auto* result = next_object_;
      next_object_ = nullptr;
      return result;
    }
    while (!stack_.empty()) {
      Position& pos = stack_.top();
      CPDF_Object* obj = pos.obj;
      ASSERT(obj->IsArray() || obj->IsDictionary() || obj->IsStream());

      if (!pos.started) {
        pos.started = true;
        if (obj->IsStream()) {
          next_object_ = obj->GetDict();
          break;
        }
        if (obj->GetDict()) {
          pos.dict_iterator = obj->GetDict()->begin();
        }
      }
      if (obj->IsStream()) {
        stack_.pop();
        continue;
      } else if (obj->IsDictionary()) {
        auto* dict = obj->GetDict();
        if (pos.dict_iterator == dict->end()) {
          stack_.pop();
          continue;
        }
        next_object_ = pos.dict_iterator->second.get();
        ++pos.dict_iterator;
        break;
      } else {
        ASSERT(obj->IsArray());
        auto* array = obj->AsArray();
        if (pos.array_pos >= array->GetCount()) {
          stack_.pop();
          continue;
        }
        next_object_ = array->GetObjectAt(pos.array_pos);
        ++pos.array_pos;
        break;
      }
    }
  }
  return nullptr;
}
