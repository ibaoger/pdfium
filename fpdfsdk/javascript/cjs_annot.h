// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_CJS_ANNOT_H_
#define FPDFSDK_JAVASCRIPT_CJS_ANNOT_H_

#include "fpdfsdk/cpdfsdk_baannot.h"
#include "fpdfsdk/javascript/JS_Define.h"

class Annot : public CJS_EmbedObj {
 public:
  explicit Annot(CJS_Object* pJSObject);
  ~Annot() override;

  CJS_Return get_hidden(CJS_Runtime* pRuntime);
  CJS_Return set_hidden(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_name(CJS_Runtime* pRuntime);
  CJS_Return set_name(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_type(CJS_Runtime* pRuntime);
  CJS_Return set_type(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  void SetSDKAnnot(CPDFSDK_BAAnnot* annot);

 private:
  CPDFSDK_Annot::ObservedPtr m_pAnnot;
};

class CJS_Annot : public CJS_Object {
 public:
  explicit CJS_Annot(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Annot() override {}

  static const char* g_pClassName;
  static int g_nObjDefnID;
  static JSPropertySpec PropertySpecs[];

  static void JSConstructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj);
  static void JSDestructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj);

  static void DefineJSObjects(CFXJS_Engine* pEngine, FXJSOBJTYPE eObjType);
  static void DefineProps(CFXJS_Engine* pEngine);

  JS_STATIC_PROP(hidden, hidden, Annot);
  JS_STATIC_PROP(name, name, Annot);
  JS_STATIC_PROP(type, type, Annot);
};

#endif  // FPDFSDK_JAVASCRIPT_CJS_ANNOT_H_
