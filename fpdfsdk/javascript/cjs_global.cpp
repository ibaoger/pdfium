// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/cjs_global.h"

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_extension.h"
#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/cjs_event_context.h"
#include "fpdfsdk/javascript/cjs_eventhandler.h"
#include "fpdfsdk/javascript/cjs_object.h"
#include "fpdfsdk/javascript/resource.h"

namespace {

enum class JS_GlobalDataType { NUMBER = 0, BOOLEAN, STRING, OBJECT, NULLOBJ };

WideString PropFromV8Prop(v8::Local<v8::String> property) {
  v8::String::Utf8Value utf8_value(property);
  return WideString::FromUTF8(ByteStringView(*utf8_value, utf8_value.length()));
}

template <class Alt>
void JSSpecialPropQuery(v8::Local<v8::String> property,
                        const v8::PropertyCallbackInfo<v8::Integer>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  if (!pJSObj)
    return;

  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  CJS_Return result = pObj->QueryProperty(PropFromV8Prop(property).c_str());
  info.GetReturnValue().Set(!result.HasError() ? 4 : 0);
}

template <class Alt>
void JSSpecialPropGet(v8::Local<v8::String> property,
                      const v8::PropertyCallbackInfo<v8::Value>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  if (!pJSObj)
    return;

  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  CJS_Return result =
      pObj->GetProperty(pRuntime, PropFromV8Prop(property).c_str());
  if (result.HasError()) {
    pRuntime->Error(
        JSFormatErrorString("global", "GetProperty", result.Error()));
    return;
  }

  if (result.HasReturn())
    info.GetReturnValue().Set(result.Return());
}

template <class Alt>
void JSSpecialPropPut(v8::Local<v8::String> property,
                      v8::Local<v8::Value> value,
                      const v8::PropertyCallbackInfo<v8::Value>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  if (!pJSObj)
    return;

  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  CJS_Return result =
      pObj->SetProperty(pRuntime, PropFromV8Prop(property).c_str(), value);
  if (result.HasError()) {
    pRuntime->Error(
        JSFormatErrorString("global", "PutProperty", result.Error()));
  }
}

template <class Alt>
void JSSpecialPropDel(v8::Local<v8::String> property,
                      const v8::PropertyCallbackInfo<v8::Boolean>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  if (!pJSObj)
    return;

  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  CJS_Return result =
      pObj->DelProperty(pRuntime, PropFromV8Prop(property).c_str());
  if (result.HasError()) {
    // TODO(dsinclair): Should this set the pRuntime->Error result?
    // ByteString cbName;
    // cbName.Format("%s.%s", "global", "DelProperty");
  }
}

struct JSGlobalData {
  JSGlobalData();
  ~JSGlobalData();

  JS_GlobalDataType nType;
  double dData;
  bool bData;
  ByteString sData;
  v8::Global<v8::Object> pData;
  bool bDeleted;
};

class JSGlobalAlternate : public CJS_EmbedObj {
 public:
  explicit JSGlobalAlternate(CJS_Object* pJSObject);
  ~JSGlobalAlternate() override;

  CJS_Return setPersistent(CJS_Runtime* pRuntime,
                           const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return QueryProperty(const wchar_t* propname);
  CJS_Return GetProperty(CJS_Runtime* pRuntime, const wchar_t* propname);
  CJS_Return SetProperty(CJS_Runtime* pRuntime,
                         const wchar_t* propname,
                         v8::Local<v8::Value> vp);
  CJS_Return DelProperty(CJS_Runtime* pRuntime, const wchar_t* propname);

 private:
  CJS_Return SetGlobalVariables(const ByteString& propname,
                                JS_GlobalDataType nType,
                                double dData,
                                bool bData,
                                const ByteString& sData,
                                v8::Local<v8::Object> pData);

  std::map<ByteString, std::unique_ptr<JSGlobalData>> m_MapGlobal;
};

}  // namespace

const JSMethodSpec CJS_Global::MethodSpecs[] = {
    {"setPersistent", setPersistent_static},
    {0, 0}};

int CJS_Global::ObjDefnID = -1;

// static
void CJS_Global::setPersistent_static(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  JSMethod<JSGlobalAlternate, &JSGlobalAlternate::setPersistent>(
      "setPersistent", "global", info);
}

// static
void CJS_Global::queryprop_static(
    v8::Local<v8::String> property,
    const v8::PropertyCallbackInfo<v8::Integer>& info) {
  JSSpecialPropQuery<JSGlobalAlternate>(property, info);
}

// static
void CJS_Global::getprop_static(
    v8::Local<v8::String> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  JSSpecialPropGet<JSGlobalAlternate>(property, info);
}

// static
void CJS_Global::putprop_static(
    v8::Local<v8::String> property,
    v8::Local<v8::Value> value,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  JSSpecialPropPut<JSGlobalAlternate>(property, value, info);
}

// static
void CJS_Global::delprop_static(
    v8::Local<v8::String> property,
    const v8::PropertyCallbackInfo<v8::Boolean>& info) {
  JSSpecialPropDel<JSGlobalAlternate>(property, info);
}

// static
void CJS_Global::DefineAllProperties(CFXJS_Engine* pEngine) {
  pEngine->DefineObjAllProperties(
      ObjDefnID, CJS_Global::queryprop_static, CJS_Global::getprop_static,
      CJS_Global::putprop_static, CJS_Global::delprop_static);
}

// static
void CJS_Global::DefineJSObjects(CFXJS_Engine* pEngine) {
  ObjDefnID = pEngine->DefineObj("global", FXJSOBJTYPE_STATIC,
                                 JSConstructor<CJS_Global, JSGlobalAlternate>,
                                 JSDestructor<CJS_Global>);
  DefineMethods(pEngine, ObjDefnID, MethodSpecs);
  DefineAllProperties(pEngine);
}

JSGlobalData::JSGlobalData()
    : nType(JS_GlobalDataType::NUMBER),
      dData(0),
      bData(false),
      sData(""),
      bDeleted(false) {}

JSGlobalData::~JSGlobalData() {
  pData.Reset();
}

JSGlobalAlternate::JSGlobalAlternate(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject) {}

JSGlobalAlternate::~JSGlobalAlternate() {}

CJS_Return JSGlobalAlternate::QueryProperty(const wchar_t* propname) {
  return CJS_Return(WideString(propname) != L"setPersistent");
}

CJS_Return JSGlobalAlternate::DelProperty(CJS_Runtime* pRuntime,
                                          const wchar_t* propname) {
  auto it = m_MapGlobal.find(ByteString::FromUnicode(propname));
  if (it == m_MapGlobal.end())
    return CJS_Return(false);

  it->second->bDeleted = true;
  return CJS_Return(true);
}

CJS_Return JSGlobalAlternate::GetProperty(CJS_Runtime* pRuntime,
                                          const wchar_t* propname) {
  auto it = m_MapGlobal.find(ByteString::FromUnicode(propname));
  if (it == m_MapGlobal.end())
    return CJS_Return(true);

  JSGlobalData* pData = it->second.get();
  if (pData->bDeleted)
    return CJS_Return(true);

  switch (pData->nType) {
    case JS_GlobalDataType::NUMBER:
      return CJS_Return(pRuntime->NewNumber(pData->dData));
    case JS_GlobalDataType::BOOLEAN:
      return CJS_Return(pRuntime->NewBoolean(pData->bData));
    case JS_GlobalDataType::STRING:
      return CJS_Return(pRuntime->NewString(
          WideString::FromLocal(pData->sData.c_str()).c_str()));
    case JS_GlobalDataType::OBJECT:
      return CJS_Return(
          v8::Local<v8::Object>::New(pRuntime->GetIsolate(), pData->pData));
    case JS_GlobalDataType::NULLOBJ:
      return CJS_Return(pRuntime->NewNull());
    default:
      break;
  }
  return CJS_Return(false);
}

CJS_Return JSGlobalAlternate::SetProperty(CJS_Runtime* pRuntime,
                                          const wchar_t* propname,
                                          v8::Local<v8::Value> vp) {
  ByteString sPropName = ByteString::FromUnicode(propname);
  if (vp->IsNumber()) {
    return SetGlobalVariables(sPropName, JS_GlobalDataType::NUMBER,
                              pRuntime->ToDouble(vp), false, "",
                              v8::Local<v8::Object>());
  }
  if (vp->IsBoolean()) {
    return SetGlobalVariables(sPropName, JS_GlobalDataType::BOOLEAN, 0,
                              pRuntime->ToBoolean(vp), "",
                              v8::Local<v8::Object>());
  }
  if (vp->IsString()) {
    return SetGlobalVariables(
        sPropName, JS_GlobalDataType::STRING, 0, false,
        ByteString::FromUnicode(pRuntime->ToWideString(vp)),
        v8::Local<v8::Object>());
  }
  if (vp->IsObject()) {
    return SetGlobalVariables(sPropName, JS_GlobalDataType::OBJECT, 0, false,
                              "", pRuntime->ToObject(vp));
  }
  if (vp->IsNull()) {
    return SetGlobalVariables(sPropName, JS_GlobalDataType::NULLOBJ, 0, false,
                              "", v8::Local<v8::Object>());
  }
  if (vp->IsUndefined()) {
    DelProperty(pRuntime, propname);
    return CJS_Return(true);
  }
  return CJS_Return(false);
}

CJS_Return JSGlobalAlternate::setPersistent(
    CJS_Runtime* pRuntime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 2)
    return CJS_Return(JSGetStringFromID(IDS_STRING_JSPARAMERROR));

  auto it = m_MapGlobal.find(
      ByteString::FromUnicode(pRuntime->ToWideString(params[0])));
  if (it == m_MapGlobal.end() || it->second->bDeleted)
    return CJS_Return(JSGetStringFromID(IDS_STRING_JSNOGLOBAL));

  // Persistent values not supported.

  return CJS_Return(true);
}

CJS_Return JSGlobalAlternate::SetGlobalVariables(const ByteString& propname,
                                                 JS_GlobalDataType nType,
                                                 double dData,
                                                 bool bData,
                                                 const ByteString& sData,
                                                 v8::Local<v8::Object> pData) {
  if (propname.IsEmpty())
    return CJS_Return(false);

  auto it = m_MapGlobal.find(propname);
  if (it != m_MapGlobal.end()) {
    JSGlobalData* pTemp = it->second.get();
    if (pTemp->bDeleted || pTemp->nType != nType) {
      pTemp->dData = 0;
      pTemp->bData = 0;
      pTemp->sData = "";
      pTemp->nType = nType;
    }
    pTemp->bDeleted = false;
    switch (nType) {
      case JS_GlobalDataType::NUMBER:
        pTemp->dData = dData;
        break;
      case JS_GlobalDataType::BOOLEAN:
        pTemp->bData = bData;
        break;
      case JS_GlobalDataType::STRING:
        pTemp->sData = sData;
        break;
      case JS_GlobalDataType::OBJECT:
        pTemp->pData.Reset(pData->GetIsolate(), pData);
        break;
      case JS_GlobalDataType::NULLOBJ:
        break;
      default:
        return CJS_Return(false);
    }
    return CJS_Return(true);
  }

  auto pNewData = pdfium::MakeUnique<JSGlobalData>();
  switch (nType) {
    case JS_GlobalDataType::NUMBER:
      pNewData->nType = JS_GlobalDataType::NUMBER;
      pNewData->dData = dData;
      break;
    case JS_GlobalDataType::BOOLEAN:
      pNewData->nType = JS_GlobalDataType::BOOLEAN;
      pNewData->bData = bData;
      break;
    case JS_GlobalDataType::STRING:
      pNewData->nType = JS_GlobalDataType::STRING;
      pNewData->sData = sData;
      break;
    case JS_GlobalDataType::OBJECT:
      pNewData->nType = JS_GlobalDataType::OBJECT;
      pNewData->pData.Reset(pData->GetIsolate(), pData);
      break;
    case JS_GlobalDataType::NULLOBJ:
      pNewData->nType = JS_GlobalDataType::NULLOBJ;
      break;
    default:
      return CJS_Return(false);
  }
  m_MapGlobal[propname] = std::move(pNewData);
  return CJS_Return(true);
}
