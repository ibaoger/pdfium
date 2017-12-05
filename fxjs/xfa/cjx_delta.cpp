// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_delta.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_delta.h"

const CJX_MethodSpec CJX_Delta::MethodSpecs[] = {{"restore", restore_static},
                                                 {"", nullptr}};

CJX_Delta::CJX_Delta(CXFA_Delta* delta) : CJX_Object(delta) {}

CJX_Delta::~CJX_Delta() {}

bool CJX_Delta::HasMethod(const WideString& func) const {
  if (HasMethodFromSpecs(MethodSpecs, func))
    return true;
  return CJX_Object::HasMethod(func);
}

void CJX_Delta::RunMethod(const WideString& func, CFXJSE_Arguments* args) {
  if (RunMethodFromSpecs(MethodSpecs, func, args))
    return;
  CJX_Object::RunMethod(func, args);
}

void CJX_Delta::restore(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"restore");
}
