// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfx_barcode.h"

#include <memory>

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/mock_ifx_renderdevice.h"
#include "testing/test_support.h"
#include "third_party/base/ptr_util.h"

class BarcodeTest : public testing::Test {
 public:
  void SetUp() override { barcode_ = pdfium::MakeUnique<CFX_Barcode>(); }

  void TearDown() override { barcode_.reset(); }

  bool Create(BC_TYPE type) {
    if (!barcode_->Create(type))
      return false;

    barcode_->SetModuleHeight(300);
    barcode_->SetModuleWidth(420);
    barcode_->SetHeight(298);
    barcode_->SetWidth(418);
    return true;
  }

  std::unique_ptr<CFX_Barcode> barcode_;
};

TEST_F(BarcodeTest, Code39) {
  EXPECT_TRUE(Create(BC_CODE39));
}
