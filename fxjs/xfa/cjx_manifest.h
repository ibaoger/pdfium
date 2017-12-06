// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_MANIFEST_H_
#define FXJS_XFA_CJX_MANIFEST_H_

#include "core/fxcrt/widestring.h"
#include "fxjs/CJX_Define.h"
#include "fxjs/cjx_node.h"

class CXFA_Manifest;
class CFXJSE_Arguments;

class CJX_Manifest : public CJX_Node {
 public:
  explicit CJX_Manifest(CXFA_Manifest* manifest);
  ~CJX_Manifest() override;

  // CJX_Object
  bool HasMethod(const WideString& func) const override;
  void RunMethod(const WideString& func, CFXJSE_Arguments* args) override;

  JS_METHOD(evaluate, CJX_Manifest);

 private:
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_MANIFEST_H_
