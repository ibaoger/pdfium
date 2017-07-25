// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_object_walker.h"

#include <sstream>
#include <string>
#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_boolean.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_null.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

namespace {

std::string Walk(CPDF_Object* object) {
  std::ostringstream result;
  CPDF_ObjectWalker walker(object);
  while (CPDF_Object* obj = walker.GetNext()) {
    if (obj->IsDictionary())
      result << " Dict";
    else if (obj->IsArray())
      result << " Arr";
    else if (obj->IsString())
      result << " Str";
    else if (obj->IsBoolean())
      result << " Bool";
    else if (obj->IsStream())
      result << " Stream";
    else if (obj->IsReference())
      result << " Ref";
    else
      result << " Unknown";
  }
  std::string result_str = result.str();
  if (!result_str.empty()) {
    result_str.erase(result_str.begin());  // remove start space
  }
  return result_str;
}

}  // namespace

TEST(cpdf_object_walker, Simple) {
  EXPECT_EQ(Walk(pdfium::MakeUnique<CPDF_Null>().get()), "Unknown");
  EXPECT_EQ(Walk(pdfium::MakeUnique<CPDF_Dictionary>().get()), "Dict");
  EXPECT_EQ(Walk(pdfium::MakeUnique<CPDF_Array>().get()), "Arr");
  EXPECT_EQ(Walk(pdfium::MakeUnique<CPDF_String>().get()), "Str");
  EXPECT_EQ(Walk(pdfium::MakeUnique<CPDF_Boolean>().get()), "Bool");
  EXPECT_EQ(Walk(pdfium::MakeUnique<CPDF_Stream>().get()), "Stream");
  EXPECT_EQ(Walk(pdfium::MakeUnique<CPDF_Reference>(nullptr, 0).get()), "Ref");
}

TEST(cpdf_object_walker, CombinedObject) {
  auto dict = pdfium::MakeUnique<CPDF_Dictionary>();
  dict->SetFor("1", pdfium::MakeUnique<CPDF_String>());
  dict->SetFor("2", pdfium::MakeUnique<CPDF_Boolean>());
  auto array = pdfium::MakeUnique<CPDF_Array>();
  array->Add(pdfium::MakeUnique<CPDF_Reference>(nullptr, 0));
  array->Add(pdfium::MakeUnique<CPDF_Null>());
  array->Add(pdfium::MakeUnique<CPDF_Stream>(
      nullptr, 0, pdfium::MakeUnique<CPDF_Dictionary>()));
  dict->SetFor("3", std::move(array));
  EXPECT_EQ(Walk(dict.get()), "Dict Str Bool Arr Ref Unknown Stream Dict");
}
