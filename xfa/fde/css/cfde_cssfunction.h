// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSFUNCTION_H_
#define XFA_FDE_CSS_CFDE_CSSFUNCTION_H_

#include "core/fxcrt/fx_string.h"
#include "xfa/fde/css/cfde_cssvaluelist.h"

class CFDE_CSSFunction {
 public:
  CFDE_CSSFunction(const CFX_WideString& funcName,
                   CFX_RetainPtr<CFDE_CSSValueList> pArgList);
  ~CFDE_CSSFunction();

  int32_t CountArgs() const { return arg_list->CountValues(); }

  CFDE_CSSValue* GetArg(int32_t index) const {
    return arg_list->GetValue(index);
  }

  const CFX_WideString GetFunctionName() const { return function_name_; }

 protected:
  CFX_RetainPtr<CFDE_CSSValueList> arg_list;
  const CFX_WideString function_name_;
};

#endif  // XFA_FDE_CSS_CFDE_CSSFUNCTION_H_
