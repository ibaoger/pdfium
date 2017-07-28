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

TEST(CPDF_ObjectWalkerTest, Simple) {
  EXPECT_EQ(Walk(pdfium::MakeUnique<CPDF_Null>().get()), "Unknown");
  EXPECT_EQ(Walk(pdfium::MakeUnique<CPDF_Dictionary>().get()), "Dict");
  EXPECT_EQ(Walk(pdfium::MakeUnique<CPDF_Array>().get()), "Arr");
  EXPECT_EQ(Walk(pdfium::MakeUnique<CPDF_String>().get()), "Str");
  EXPECT_EQ(Walk(pdfium::MakeUnique<CPDF_Boolean>().get()), "Bool");
  EXPECT_EQ(Walk(pdfium::MakeUnique<CPDF_Stream>().get()), "Stream");
  EXPECT_EQ(Walk(pdfium::MakeUnique<CPDF_Reference>(nullptr, 0).get()), "Ref");
}

TEST(CPDF_ObjectWalkerTest, CombinedObject) {
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

TEST(CPDF_ObjectWalkerTest, GetParent) {
  auto level_4 = pdfium::MakeUnique<CPDF_Null>();
  auto level_3 = pdfium::MakeUnique<CPDF_Dictionary>();
  level_3->SetFor("AnyObj", std::move(level_4));
  auto level_2 =
      pdfium::MakeUnique<CPDF_Stream>(nullptr, 0, std::move(level_3));
  auto level_1 = pdfium::MakeUnique<CPDF_Array>();
  level_1->Add(std::move(level_2));
  auto level_0 = pdfium::MakeUnique<CPDF_Dictionary>();
  level_0->SetFor("Array", std::move(level_1));

  // We have <</Array [ stream( << .AnyObj null >>) ]>>
  // In this case each step will increase depth.
  // And we each prev object should be parent for current.
  CPDF_Object* cur_parent = nullptr;
  CPDF_ObjectWalker walker(level_0.get());
  while (CPDF_Object* obj = walker.GetNext()) {
    EXPECT_EQ(cur_parent, walker.GetParent());
    cur_parent = obj;
  }
}

TEST(CPDF_ObjectWalkerTest, SkipWalkIntoCurrentObject) {
  auto root_array = pdfium::MakeUnique<CPDF_Array>();
  root_array->AddNew<CPDF_Null>();
  root_array->AddNew<CPDF_Null>();
  // now object contains 2 child null objects.
  root_array->Add(root_array->Clone());
  // now object contains 4 child null objects.

  int non_array_objects = 0;
  CPDF_ObjectWalker walker(root_array.get());
  while (CPDF_Object* obj = walker.GetNext()) {
    if (obj != root_array.get() && obj->IsArray()) {
      // skip other array except root.
      walker.SkipWalkIntoCurrentObject();
    }
    if (!obj->IsArray())
      ++non_array_objects;
  }
  // 2 objects from child array should be skipped.
  EXPECT_EQ(2, non_array_objects);
}
