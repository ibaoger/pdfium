// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/cfx_unowned_ptr.h"

#include <utility>
#include <vector>

#include "testing/fx_string_testhelpers.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class Clink {
 public:
  CFX_UnownedPtr<Clink> next_ = nullptr;
};

}  // namespace

TEST(fxcrt, UnownedPtrOk) {
  Clink* ptr1 = new Clink();
  Clink* ptr2 = new Clink();
  ptr2->next_ = ptr1;
  delete ptr2;
  delete ptr1;
}

TEST(fxcrt, UnownedPtrNotOk) {
  Clink* ptr1 = new Clink();
  Clink* ptr2 = new Clink();
  ptr1->next_ = ptr2;
  delete ptr2;
#if defined(MEMORY_TOOL_REPLACES_ALLOCATOR)
  EXPECT_DEATH(delete ptr1, "");
#else
  delete ptr1;
#endif
}
