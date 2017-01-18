// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSFUNCTIONVALUE_H_
#define XFA_FDE_CSS_CFDE_CSSFUNCTIONVALUE_H_

#include <memory>

#include "xfa/fde/css/cfde_cssvalue.h"

class CFDE_CSSFunction;

class CFDE_CSSFunctionValue : public CFDE_CSSValue {
 public:
  explicit CFDE_CSSFunctionValue(std::unique_ptr<CFDE_CSSFunction> value);
  ~CFDE_CSSFunctionValue() override;

  const CFX_WideString GetFunctionName() const;

  int32_t CountArgs() const;
  CFDE_CSSValue* GetArg(int32_t index) const;

 private:
  std::unique_ptr<CFDE_CSSFunction> value_;
};

#endif  // XFA_FDE_CSS_CFDE_CSSFUNCTIONVALUE_H_
