// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_random.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(FX_Random, GenerateMT100times) {
  // Prove this doesn't spin wait for seconds at a time.
  uint32_t waste[16];
  for (int i = 0; i < 100; ++i) {
    FX_Random_GenerateMT(waste, 16);
  }
}
