// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/cjs_scalewhen.h"

JSConstSpec CJS_ScaleWhen::ConstSpecs[] = {
    {"always", JSConstSpec::Number, 0, 0},
    {"never", JSConstSpec::Number, 1, 0},
    {"tooBig", JSConstSpec::Number, 2, 0},
    {"tooSmall", JSConstSpec::Number, 3, 0},
    {0, JSConstSpec::Number, 0, 0}};

const char* CJS_ScaleWhen::g_pClassName = "scaleWhen";
int CJS_ScaleWhen::g_nObjDefnID = -1;

void CJS_ScaleWhen::DefineConsts(CFXJS_Engine* pEngine) {
  for (size_t i = 0; i < FX_ArraySize(ConstSpecs) - 1; ++i) {
    pEngine->DefineObjConst(
        g_nObjDefnID, ConstSpecs[i].pName,
        ConstSpecs[i].eType == JSConstSpec::Number
            ? pEngine->NewNumber(ConstSpecs[i].number).As<v8::Value>()
            : pEngine->NewString(ConstSpecs[i].pStr).As<v8::Value>());
  }
}

void CJS_ScaleWhen::DefineJSObjects(CFXJS_Engine* pEngine,
                                    FXJSOBJTYPE eObjType) {
  g_nObjDefnID = pEngine->DefineObj(CJS_ScaleWhen::g_pClassName, eObjType,
                                    nullptr, nullptr);
  DefineConsts(pEngine);
}
