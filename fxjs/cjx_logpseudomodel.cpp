// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjx_logpseudomodel.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cscript_logpseudomodel.h"

const CJX_MethodSpec CJX_LogPseudoModel::MethodSpecs[] = {
    {"message", message_static},
    {"traceEnabled", traceEnabled_static},
    {"traceActivate", traceActivate_static},
    {"traceDeactivate", traceDeactivate_static},
    {"trace", trace_static},
    {"", nullptr}};

CJX_LogPseudoModel::CJX_LogPseudoModel(CScript_LogPseudoModel* model)
    : CJX_Object(model) {}

CJX_LogPseudoModel::~CJX_LogPseudoModel() {}

bool CJX_LogPseudoModel::HasMethod(const WideString& func) const {
  if (HasMethodFromSpecs(MethodSpecs, func))
    return true;
  return CJX_Object::HasMethod(func);
}

void CJX_LogPseudoModel::RunMethod(const WideString& func,
                                   CFXJSE_Arguments* args) {
  if (RunMethodFromSpecs(MethodSpecs, func, args))
    return;
  CJX_Object::RunMethod(func, args);
}

void CJX_LogPseudoModel::message(CFXJSE_Arguments* pArguments) {}

void CJX_LogPseudoModel::traceEnabled(CFXJSE_Arguments* pArguments) {}

void CJX_LogPseudoModel::traceActivate(CFXJSE_Arguments* pArguments) {}

void CJX_LogPseudoModel::traceDeactivate(CFXJSE_Arguments* pArguments) {}

void CJX_LogPseudoModel::trace(CFXJSE_Arguments* pArguments) {}
