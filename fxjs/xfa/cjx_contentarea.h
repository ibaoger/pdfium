// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_CONTENTAREA_H_
#define FXJS_XFA_CJX_CONTENTAREA_H_

#include "fxjs/xfa/cjx_container.h"

class CXFA_ContentArea;

class CJX_ContentArea : public CJX_Container {
 public:
  explicit CJX_ContentArea(CXFA_ContentArea* node);
  ~CJX_ContentArea() override;

  JS_PROP(relevant);
  JS_PROP(use);
  JS_PROP(usehref);
  JS_PROP(x);
  JS_PROP(y);
};

#endif  // FXJS_XFA_CJX_CONTENTAREA_H_
