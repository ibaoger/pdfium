// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_indirect_object_holder.h"

#include <memory>
#include <utility>

#include "core/fpdfapi/parser/cpdf_null.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

namespace {

class CPDF_IndirectObjectHolderWithRecursiveSameParse
    : public CPDF_IndirectObjectHolder {
 public:
  CPDF_IndirectObjectHolderWithRecursiveSameParse() {}
  ~CPDF_IndirectObjectHolderWithRecursiveSameParse() override {}

  std::unique_ptr<CPDF_Object> ParseIndirectObject(uint32_t objnum) override {
    const CPDF_Object* same_parse = GetOrParseIndirectObject(objnum);
    CHECK(!same_parse);
    return pdfium::MakeUnique<CPDF_Null>();
  }
};

}  // namespace

TEST(CPDF_IndirectObjectHolderTest, RecursiveParseOfSameObject) {
  CPDF_IndirectObjectHolderWithRecursiveSameParse test_holder;

  EXPECT_TRUE(test_holder.GetOrParseIndirectObject(1000));
}
