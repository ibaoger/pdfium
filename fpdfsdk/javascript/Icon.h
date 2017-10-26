// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_ICON_H_
#define FPDFSDK_JAVASCRIPT_ICON_H_

#include "fpdfsdk/javascript/JS_Define.h"

class CPDF_Stream;

class Icon : public CJS_EmbedObj {
 public:
  explicit Icon(CJS_Object* pJSObject);
  ~Icon() override;

  CJS_Return get_name(CJS_Runtime* pRuntime);
  CJS_Return set_name(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  WideString GetIconName() const { return m_swIconName; }
  void SetIconName(WideString name) { m_swIconName = name; }

 private:
  WideString m_swIconName;
};

class CJS_Icon : public CJS_Object {
 public:
  static int GetObjId();
  static void DefineJSObjects(CFXJS_Engine* pEngine);

  explicit CJS_Icon(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Icon() override {}

  JS_STATIC_PROP(name, name, Icon);

 private:
  static const JSPropertySpec PropertySpecs[];
};

#endif  // FPDFSDK_JAVASCRIPT_ICON_H_
