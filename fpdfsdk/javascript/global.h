// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_GLOBAL_H_
#define FPDFSDK_JAVASCRIPT_GLOBAL_H_

#include <map>
#include <memory>
#include <vector>

#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_KeyValue.h"

class CJS_GlobalData;
class CJS_GlobalVariableArray;
class CJS_KeyValue;

struct JSGlobalData {
  JSGlobalData();
  ~JSGlobalData();

  JS_GlobalDataType nType;
  double dData;
  bool bData;
  ByteString sData;
  v8::Global<v8::Object> pData;
  bool bPersistent;
  bool bDeleted;
};

class JSGlobalAlternate : public CJS_EmbedObj {
 public:
  explicit JSGlobalAlternate(CJS_Object* pJSObject);
  ~JSGlobalAlternate() override;

  bool setPersistent(CJS_Runtime* pRuntime,
                     const std::vector<CJS_Value>& params,
                     CJS_Value& vRet,
                     WideString& sError);
  bool QueryProperty(const wchar_t* propname);
  bool GetProperty(CJS_Runtime* pRuntime,
                   const wchar_t* propname,
                   CJS_Value* vp);
  bool SetProperty(CJS_Runtime* pRuntime,
                   const wchar_t* propname,
                   const CJS_Value& vp);
  bool DelProperty(CJS_Runtime* pRuntime, const wchar_t* propname);
  void Initial(CPDFSDK_FormFillEnvironment* pFormFillEnv);

 private:
  void UpdateGlobalPersistentVariables();
  void CommitGlobalPersisitentVariables(CJS_Runtime* pRuntime);
  void DestroyGlobalPersisitentVariables();
  bool SetGlobalVariables(const ByteString& propname,
                          JS_GlobalDataType nType,
                          double dData,
                          bool bData,
                          const ByteString& sData,
                          v8::Local<v8::Object> pData,
                          bool bDefaultPersistent);
  void ObjectToArray(CJS_Runtime* pRuntime,
                     v8::Local<v8::Object> pObj,
                     CJS_GlobalVariableArray& array);
  void PutObjectProperty(v8::Local<v8::Object> obj, CJS_KeyValue* pData);

  std::map<ByteString, std::unique_ptr<JSGlobalData>> m_MapGlobal;
  WideString m_sFilePath;
  CJS_GlobalData* m_pGlobalData;
  CPDFSDK_FormFillEnvironment::ObservedPtr m_pFormFillEnv;
};

class CJS_Global : public CJS_Object {
 public:
  explicit CJS_Global(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Global() override {}

  // CJS_Object
  void InitInstance(IJS_Runtime* pIRuntime) override;

  DECLARE_SPECIAL_JS_CLASS();
  JS_SPECIAL_STATIC_METHOD(setPersistent, JSGlobalAlternate, global);
};

#endif  // FPDFSDK_JAVASCRIPT_GLOBAL_H_
