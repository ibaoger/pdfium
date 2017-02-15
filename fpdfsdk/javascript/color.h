// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_COLOR_H_
#define FPDFSDK_JAVASCRIPT_COLOR_H_

#include <vector>

#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/pdfwindow/PWL_Wnd.h"

class color : public CJS_EmbedObj {
 public:
  explicit color(CJS_Object* pJSObject);
  ~color() override;

  bool black(IJS_EventContext* cc, CJS_PropValue& vp, CFX_WideString& sError);
  bool blue(IJS_EventContext* cc, CJS_PropValue& vp, CFX_WideString& sError);
  bool cyan(IJS_EventContext* cc, CJS_PropValue& vp, CFX_WideString& sError);
  bool dkGray(IJS_EventContext* cc, CJS_PropValue& vp, CFX_WideString& sError);
  bool gray(IJS_EventContext* cc, CJS_PropValue& vp, CFX_WideString& sError);
  bool green(IJS_EventContext* cc, CJS_PropValue& vp, CFX_WideString& sError);
  bool ltGray(IJS_EventContext* cc, CJS_PropValue& vp, CFX_WideString& sError);
  bool magenta(IJS_EventContext* cc, CJS_PropValue& vp, CFX_WideString& sError);
  bool red(IJS_EventContext* cc, CJS_PropValue& vp, CFX_WideString& sError);
  bool transparent(IJS_EventContext* cc,
                   CJS_PropValue& vp,
                   CFX_WideString& sError);
  bool white(IJS_EventContext* cc, CJS_PropValue& vp, CFX_WideString& sError);
  bool yellow(IJS_EventContext* cc, CJS_PropValue& vp, CFX_WideString& sError);

  bool convert(IJS_EventContext* cc,
               const std::vector<CJS_Value>& params,
               CJS_Value& vRet,
               CFX_WideString& sError);
  bool equal(IJS_EventContext* cc,
             const std::vector<CJS_Value>& params,
             CJS_Value& vRet,
             CFX_WideString& sError);

  static void ConvertPWLColorToArray(CJS_Runtime* pRuntime,
                                     const CPWL_Color& color,
                                     CJS_Array* array);
  static void ConvertArrayToPWLColor(CJS_Runtime* pRuntime,
                                     const CJS_Array& array,
                                     CPWL_Color* color);

 private:
  CPWL_Color m_crTransparent;
  CPWL_Color m_crBlack;
  CPWL_Color m_crWhite;
  CPWL_Color m_crRed;
  CPWL_Color m_crGreen;
  CPWL_Color m_crBlue;
  CPWL_Color m_crCyan;
  CPWL_Color m_crMagenta;
  CPWL_Color m_crYellow;
  CPWL_Color m_crDKGray;
  CPWL_Color m_crGray;
  CPWL_Color m_crLTGray;
};

class CJS_Color : public CJS_Object {
 public:
  explicit CJS_Color(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Color() override {}

  DECLARE_JS_CLASS();

  JS_STATIC_PROP(black, color);
  JS_STATIC_PROP(blue, color);
  JS_STATIC_PROP(cyan, color);
  JS_STATIC_PROP(dkGray, color);
  JS_STATIC_PROP(gray, color);
  JS_STATIC_PROP(green, color);
  JS_STATIC_PROP(ltGray, color);
  JS_STATIC_PROP(magenta, color);
  JS_STATIC_PROP(red, color);
  JS_STATIC_PROP(transparent, color);
  JS_STATIC_PROP(white, color);
  JS_STATIC_PROP(yellow, color);

  JS_STATIC_METHOD(convert, color);
  JS_STATIC_METHOD(equal, color);
};

#endif  // FPDFSDK_JAVASCRIPT_COLOR_H_
