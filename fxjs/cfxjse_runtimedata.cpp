// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cfxjse_runtimedata.h"

#include <utility>

#include "fxjs/cfxjse_isolatetracker.h"
#include "fxjs/fxjs_v8.h"

CFXJSE_RuntimeData::CFXJSE_RuntimeData(v8::Isolate* pIsolate)
    : m_pIsolate(pIsolate) {}

CFXJSE_RuntimeData::~CFXJSE_RuntimeData() {}

std::unique_ptr<CFXJSE_RuntimeData> CFXJSE_RuntimeData::Create(
    v8::Isolate* pIsolate) {
  std::unique_ptr<CFXJSE_RuntimeData> pRuntimeData(
      new CFXJSE_RuntimeData(pIsolate));
  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);

  v8::Local<v8::FunctionTemplate> hFuncTemplate =
      v8::FunctionTemplate::New(pIsolate);
  v8::Local<v8::ObjectTemplate> hGlobalTemplate =
      hFuncTemplate->InstanceTemplate();
  hGlobalTemplate->Set(
      v8::Symbol::GetToStringTag(pIsolate),
      v8::String::NewFromUtf8(pIsolate, "global", v8::NewStringType::kNormal)
          .ToLocalChecked());

  v8::Local<v8::Context> hContext =
      v8::Context::New(pIsolate, 0, hGlobalTemplate);
  hContext->SetSecurityToken(v8::External::New(pIsolate, pIsolate));

  pRuntimeData->m_hRootContextGlobalTemplate.Reset(pIsolate, hFuncTemplate);
  pRuntimeData->m_hRootContext.Reset(pIsolate, hContext);
  return pRuntimeData;
}

CFXJSE_RuntimeData* CFXJSE_RuntimeData::Get(v8::Isolate* pIsolate) {
  FXJS_PerIsolateData::SetUp(pIsolate);

  FXJS_PerIsolateData* pData = FXJS_PerIsolateData::Get(pIsolate);
  if (!pData->m_pFXJSERuntimeData)
    pData->m_pFXJSERuntimeData = CFXJSE_RuntimeData::Create(pIsolate);
  return pData->m_pFXJSERuntimeData.get();
}
