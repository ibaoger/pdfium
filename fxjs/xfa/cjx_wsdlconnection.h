// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_WSDLCONNECTION_H_
#define FXJS_XFA_CJX_WSDLCONNECTION_H_

#include "core/fxcrt/widestring.h"
#include "fxjs/CJX_Define.h"
#include "fxjs/cjx_node.h"

class CXFA_WsdlConnection;
class CFXJSE_Arguments;

class CJX_WsdlConnection : public CJX_Node {
 public:
  explicit CJX_WsdlConnection(CXFA_WsdlConnection* connection);
  ~CJX_WsdlConnection() override;

  // CJX_Object
  bool HasMethod(const WideString& func) const override;
  void RunMethod(const WideString& func, CFXJSE_Arguments* args) override;

  JS_METHOD(execute, CJX_WsdlConnection);

 private:
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_WSDLCONNECTION_H_
