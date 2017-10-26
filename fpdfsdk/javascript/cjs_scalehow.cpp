// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/cjs_scalehow.h"

namespace {

int g_scaleHowObjId = -1;

}  // namespace

const JSConstSpec CJS_ScaleHow::ConstSpecs[] = {
    {"proportional", JSConstSpec::Number, 0, 0},
    {"anamorphic", JSConstSpec::Number, 1, 0},
    {0, JSConstSpec::Number, 0, 0}};

// static
void CJS_ScaleHow::DefineJSObjects(CFXJS_Engine* pEngine) {
  g_scaleHowObjId =
      pEngine->DefineObj("scaleHow", FXJSOBJTYPE_STATIC, nullptr, nullptr);
  DefineConsts(pEngine, g_scaleHowObjId, ConstSpecs);
}
