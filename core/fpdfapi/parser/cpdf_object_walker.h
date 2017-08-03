// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FPDFAPI_PARSER_CPDF_OBJECT_WALKER_H_
#define CORE_FPDFAPI_PARSER_CPDF_OBJECT_WALKER_H_

#include <memory>
#include <stack>

#include "core/fpdfapi/parser/cpdf_dictionary.h"

// Walk on all non-null sub-objects in an object in depth, include itself,
// like in flat list.
class CPDF_ObjectWalker {
 public:
  explicit CPDF_ObjectWalker(const CPDF_Object* root);
  ~CPDF_ObjectWalker();

  const CPDF_Object* GetNext();
  void SkipWalkIntoCurrentObject();

  size_t current_depth() const { return current_depth_; }
  const CPDF_Object* GetParent() const { return parent_object_; }
  size_t array_index() const { return array_index_; }
  const CFX_ByteString& dictionary_key() const { return dict_key_; }

 private:
  class ArrayIterator;
  class DictionaryIterator;
  class StreamIterator;
  class SubObjectsIterator;

  static std::unique_ptr<SubObjectsIterator> MakeIterator(
      const CPDF_Object* object);

  const CPDF_Object* next_object_;
  const CPDF_Object* parent_object_;

  CFX_ByteString dict_key_;
  size_t array_index_;
  size_t current_depth_;

  std::stack<std::unique_ptr<SubObjectsIterator>> stack_;
};

class CPDF_NonConstObjectWalker : public CPDF_ObjectWalker {
 public:
  explicit CPDF_NonConstObjectWalker(CPDF_Object* root)
      : CPDF_ObjectWalker(root) {}

  CPDF_Object* GetNext() {
    return const_cast<CPDF_Object*>(CPDF_ObjectWalker::GetNext());
  }
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_OBJECT_WALKER_H_
