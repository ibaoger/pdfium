// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/Icon.h"

#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"

BEGIN_JS_STATIC_CONST(CJS_Icon)
END_JS_STATIC_CONST()

JSPropertySpec CJS_Icon::JS_Class_Properties[] = {
    {L"name", get_name_static, set_name_static},
    {0, 0, 0}};

BEGIN_JS_STATIC_METHOD(CJS_Icon)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_Icon, Icon)

Icon::Icon(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject), m_pIconStream(nullptr), m_swIconName(L"") {}

Icon::~Icon() {}

void Icon::SetStream(CPDF_Stream* pIconStream) {
  if (pIconStream)
    m_pIconStream = pIconStream;
}

CPDF_Stream* Icon::GetStream() {
  return m_pIconStream;
}

void Icon::SetIconName(CFX_WideString name) {
  m_swIconName = name;
}

CFX_WideString Icon::GetIconName() {
  return m_swIconName;
}

bool Icon::name(IJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  vp << m_swIconName;
  return true;
}
