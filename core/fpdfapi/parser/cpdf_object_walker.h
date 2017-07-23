// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FPDFAPI_PARSER_CPDF_OBJECT_WALKER_H_
#define CORE_FPDFAPI_PARSER_CPDF_OBJECT_WALKER_H_

#include <stack>

#include "core/fpdfapi/parser/cpdf_dictionary.h"

class CPDF_ObjectWalker {
 public:
  explicit CPDF_ObjectWalker(CPDF_Object* root);
  CPDF_Object* GetNext();

 private:
  struct Position {
    CPDF_Object* obj;
    size_t array_pos;
    CPDF_Dictionary::const_iterator dict_iterator;
    bool started;
  };
  CPDF_Object* next_object_;

  std::stack<Position> stack_;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_OBJECT_WALKER_H_
