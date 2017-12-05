// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJX_SIGNATUREPSEUDOMODEL_H_
#define FXJS_CJX_SIGNATUREPSEUDOMODEL_H_

#include "fxjs/CJX_Define.h"
#include "fxjs/cjx_object.h"

class CFXJSE_Arguments;
class CFXJSE_Value;
class CScript_SignaturePseudoModel;

class CJX_SignaturePseudoModel : public CJX_Object {
 public:
  explicit CJX_SignaturePseudoModel(CScript_SignaturePseudoModel* model);
  ~CJX_SignaturePseudoModel() override;

  // CJX_Object
  bool HasMethod(const WideString& func) const override;
  void RunMethod(const WideString& func, CFXJSE_Arguments* args) override;

  JS_METHOD(verify, CJX_SignaturePseudoModel);
  JS_METHOD(sign, CJX_SignaturePseudoModel);
  JS_METHOD(enumerate, CJX_SignaturePseudoModel);
  JS_METHOD(clear, CJX_SignaturePseudoModel);

 private:
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_CJX_SIGNATUREPSEUDOMODEL_H_
