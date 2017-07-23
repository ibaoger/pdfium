// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FPDFAPI_PARSER_CPDF_OBJECT_WALKER_H_
#define CORE_FPDFAPI_PARSER_CPDF_OBJECT_WALKER_H_

#include <stack>

#include "core/fpdfapi/parser/cpdf_dictionary.h"

// Walk around all non null subobjects in object, include self, like in flat
// list.
class CPDF_ObjectWalker final {
 public:
  explicit CPDF_ObjectWalker(CPDF_Object* root);
  ~CPDF_ObjectWalker();

  CPDF_Object* GetNext();

 private:
  struct Position {
    explicit Position(CPDF_Object* obj);
    CPDF_Object* obj;
    size_t array_pos = 0;
    CPDF_Dictionary::const_iterator dict_iterator;
    bool started = false;
  };
  CPDF_Object* next_object_;

  std::stack<Position> stack_;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_OBJECT_WALKER_H_
