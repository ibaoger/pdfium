// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/app/xfa_ffbarcode.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

TEST(XFA_FFBarcode, GetBarcodeTypeByName) {
  auto acc = pdfium::MakeUnique<CXFA_WidgetAcc>(nullptr, nullptr);
  auto barcode = pdfium::MakeUnique<CXFA_FFBarcode>(acc.get());
  EXPECT_EQ(nullptr, barcode->GetBarcodeTypeByName(L""));
  EXPECT_EQ(nullptr, barcode->GetBarcodeTypeByName(L"not_found"));

  auto* data = barcode->GetBarcodeTypeByName(L"pdf417");
  ASSERT_NE(nullptr, data);
  EXPECT_EQ(BarcodeType::pdf417, data->eName);
}
