// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/cjs_zoomtype.h"

namespace {

int g_zoomtypeObjId = -1;

}  // namespace

const JSConstSpec CJS_Zoomtype::ConstSpecs[] = {
    {"none", JSConstSpec::String, 0, "NoVary"},
    {"fitP", JSConstSpec::String, 0, "FitPage"},
    {"fitW", JSConstSpec::String, 0, "FitWidth"},
    {"fitH", JSConstSpec::String, 0, "FitHeight"},
    {"fitV", JSConstSpec::String, 0, "FitVisibleWidth"},
    {"pref", JSConstSpec::String, 0, "Preferred"},
    {"refW", JSConstSpec::String, 0, "ReflowWidth"},
    {0, JSConstSpec::Number, 0, 0}};

// static
void CJS_Zoomtype::DefineJSObjects(CFXJS_Engine* pEngine) {
  g_zoomtypeObjId =
      pEngine->DefineObj("zoomtype", FXJSOBJTYPE_STATIC, nullptr, nullptr);
  DefineConsts(pEngine, g_zoomtypeObjId, ConstSpecs);
}
