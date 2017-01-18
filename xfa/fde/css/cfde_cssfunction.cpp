// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssfunction.h"

CFDE_CSSFunction::CFDE_CSSFunction(const CFX_WideString& funcName,
                                   CFX_RetainPtr<CFDE_CSSValueList> pArgList)
    : arg_list(pArgList), function_name_(funcName) {
  ASSERT(pArgList);
}

CFDE_CSSFunction::~CFDE_CSSFunction() {}
