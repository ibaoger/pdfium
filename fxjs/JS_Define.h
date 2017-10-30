// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_JS_DEFINE_H_
#define FXJS_JS_DEFINE_H_

#include <vector>

#include "fxjs/cjs_object.h"
#include "fxjs/cjs_return.h"
#include "fxjs/fxjs_v8.h"

#define IDS_STRING_JSALERT 25613
#define IDS_STRING_JSPARAMERROR 25614
#define IDS_STRING_JSAFNUMBER_KEYSTROKE 25615
#define IDS_STRING_JSPARAM_TOOLONG 25617
#define IDS_STRING_JSPARSEDATE 25618
#define IDS_STRING_JSRANGE1 25619
#define IDS_STRING_JSRANGE2 25620
#define IDS_STRING_JSRANGE3 25621
#define IDS_STRING_JSNOTSUPPORT 25627
#define IDS_STRING_JSBUSY 25628
#define IDS_STRING_JSEVENT 25629
#define IDS_STRING_RUN 25630
#define IDS_STRING_JSPRINT1 25632
#define IDS_STRING_JSPRINT2 25633
#define IDS_STRING_JSNOGLOBAL 25635
#define IDS_STRING_JSREADONLY 25636
#define IDS_STRING_JSTYPEERROR 25637
#define IDS_STRING_JSVALUEERROR 25638
#define IDS_STRING_JSNOPERMISSION 25639
#define IDS_STRING_JSBADOBJECT 25640

double JS_GetDateTime();
int JS_GetYearFromTime(double dt);
int JS_GetMonthFromTime(double dt);
int JS_GetDayFromTime(double dt);
int JS_GetHourFromTime(double dt);
int JS_GetMinFromTime(double dt);
int JS_GetSecFromTime(double dt);
double JS_LocalTime(double d);
double JS_DateParse(const WideString& str);
double JS_MakeDay(int nYear, int nMonth, int nDay);
double JS_MakeTime(int nHour, int nMin, int nSec, int nMs);
double JS_MakeDate(double day, double time);

WideString JSGetStringFromID(uint32_t id);
WideString JSFormatErrorString(const char* class_name,
                               const char* property_name,
                               const WideString& details);

// Some JS methods have the bizarre convention that they may also be called
// with a single argument which is an object containing the actual arguments
// as its properties. The varying arguments to this method are the property
// names as wchar_t string literals corresponding to each positional argument.
// The result will always contain |nKeywords| value, with unspecified ones
// being set to type VT_unknown.
std::vector<v8::Local<v8::Value>> ExpandKeywordParams(
    CJS_Runtime* pRuntime,
    const std::vector<v8::Local<v8::Value>>& originals,
    size_t nKeywords,
    ...);

// All JS classes have a name, an object defintion ID, and the ability to
// register themselves with FXJS_V8. We never make a BASE class on its own
// because it can't really do anything.

// Rich JS classes provide constants, methods, properties, and the ability
// to construct native object state.

template <class T, class A>
static void JSConstructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj) {
  CJS_Object* pObj = new T(obj);
  pObj->SetEmbedObject(new A(pObj));
  pEngine->SetObjectPrivate(obj, pObj);
  pObj->InitInstance(static_cast<CJS_Runtime*>(pEngine));
}

template <class T>
static void JSDestructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj) {
  delete static_cast<T*>(pEngine->GetObjectPrivate(obj));
}

template <class C, CJS_Return (C::*M)(CJS_Runtime*)>
void JSPropGetter(const char* prop_name_string,
                  const char* class_name_string,
                  v8::Local<v8::String> property,
                  const v8::PropertyCallbackInfo<v8::Value>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  if (!pJSObj)
    return;

  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  CJS_Return result = (pObj->*M)(pRuntime);
  if (result.HasError()) {
    pRuntime->Error(JSFormatErrorString(class_name_string, prop_name_string,
                                        result.Error()));
    return;
  }

  if (result.HasReturn())
    info.GetReturnValue().Set(result.Return());
}

template <class C, CJS_Return (C::*M)(CJS_Runtime*, v8::Local<v8::Value>)>
void JSPropSetter(const char* prop_name_string,
                  const char* class_name_string,
                  v8::Local<v8::String> property,
                  v8::Local<v8::Value> value,
                  const v8::PropertyCallbackInfo<void>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  if (!pJSObj)
    return;

  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  CJS_Return result = (pObj->*M)(pRuntime, value);
  if (result.HasError()) {
    pRuntime->Error(JSFormatErrorString(class_name_string, prop_name_string,
                                        result.Error()));
  }
}

template <class C,
          CJS_Return (C::*M)(CJS_Runtime*,
                             const std::vector<v8::Local<v8::Value>>&)>
void JSMethod(const char* method_name_string,
              const char* class_name_string,
              const v8::FunctionCallbackInfo<v8::Value>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  if (!pJSObj)
    return;

  std::vector<v8::Local<v8::Value>> parameters;
  for (unsigned int i = 0; i < (unsigned int)info.Length(); i++)
    parameters.push_back(info[i]);

  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  CJS_Return result = (pObj->*M)(pRuntime, parameters);
  if (result.HasError()) {
    pRuntime->Error(JSFormatErrorString(class_name_string, method_name_string,
                                        result.Error()));
    return;
  }

  if (result.HasReturn())
    info.GetReturnValue().Set(result.Return());
}

#define JS_STATIC_PROP(err_name, prop_name, class_name)           \
  static void get_##prop_name##_static(                           \
      v8::Local<v8::String> property,                             \
      const v8::PropertyCallbackInfo<v8::Value>& info) {          \
    JSPropGetter<class_name, &class_name::get_##prop_name>(       \
        #err_name, #class_name, property, info);                  \
  }                                                               \
  static void set_##prop_name##_static(                           \
      v8::Local<v8::String> property, v8::Local<v8::Value> value, \
      const v8::PropertyCallbackInfo<void>& info) {               \
    JSPropSetter<class_name, &class_name::set_##prop_name>(       \
        #err_name, #class_name, property, value, info);           \
  }

#define JS_STATIC_METHOD(method_name, class_name)                             \
  static void method_name##_static(                                           \
      const v8::FunctionCallbackInfo<v8::Value>& info) {                      \
    JSMethod<class_name, &class_name::method_name>(#method_name, #class_name, \
                                                   info);                     \
  }

#endif  // FXJS_JS_DEFINE_H_
