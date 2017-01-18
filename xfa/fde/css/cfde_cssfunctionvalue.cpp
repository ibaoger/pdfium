// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssfunctionvalue.h"

#include <utility>

#include "xfa/fde/css/cfde_cssfunction.h"

CFDE_CSSFunctionValue::CFDE_CSSFunctionValue(
    std::unique_ptr<CFDE_CSSFunction> value)
    : CFDE_CSSValue(FDE_CSSPrimitiveType::Function), value_(std::move(value)) {}

CFDE_CSSFunctionValue::~CFDE_CSSFunctionValue() {}

const CFX_WideString CFDE_CSSFunctionValue::GetFunctionName() const {
  return value_->GetFunctionName();
}

int32_t CFDE_CSSFunctionValue::CountArgs() const {
  return value_->CountArgs();
}

CFDE_CSSValue* CFDE_CSSFunctionValue::GetArg(int32_t index) const {
  return value_->GetArg(index);
}
