// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/cjs_scalewhen.h"

namespace {

int g_scaleWhenObjId = -1;

}  // namespace

JSConstSpec CJS_ScaleWhen::ConstSpecs[] = {
    {"always", JSConstSpec::Number, 0, 0},
    {"never", JSConstSpec::Number, 1, 0},
    {"tooBig", JSConstSpec::Number, 2, 0},
    {"tooSmall", JSConstSpec::Number, 3, 0},
    {0, JSConstSpec::Number, 0, 0}};

// static
void CJS_ScaleWhen::DefineJSObjects(CFXJS_Engine* pEngine,
                                    FXJSOBJTYPE eObjType) {
  g_scaleWhenObjId =
      pEngine->DefineObj("scaleWhen", eObjType, nullptr, nullptr);
  DefineConsts(pEngine, g_scaleWhenObjId, ConstSpecs);
}
