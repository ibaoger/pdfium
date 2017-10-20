// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_COLOR_H_
#define FPDFSDK_JAVASCRIPT_COLOR_H_

#include <vector>

#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"

class color : public CJS_EmbedObj {
 public:
  explicit color(CJS_Object* pJSObject);
  ~color() override;

  bool get_black(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_black(CJS_Runtime* pRuntime,
                 const CJS_Value& vp,
                 WideString* sError);

  bool get_blue(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_blue(CJS_Runtime* pRuntime, const CJS_Value& vp, WideString* sError);

  bool get_cyan(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_cyan(CJS_Runtime* pRuntime, const CJS_Value& vp, WideString* sError);

  bool get_dark_gray(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_dark_gray(CJS_Runtime* pRuntime,
                     const CJS_Value& vp,
                     WideString* sError);

  bool get_gray(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_gray(CJS_Runtime* pRuntime, const CJS_Value& vp, WideString* sError);

  bool get_green(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_green(CJS_Runtime* pRuntime,
                 const CJS_Value& vp,
                 WideString* sError);

  bool get_light_gray(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_light_gray(CJS_Runtime* pRuntime,
                      const CJS_Value& vp,
                      WideString* sError);

  bool get_magenta(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_magenta(CJS_Runtime* pRuntime,
                   const CJS_Value& vp,
                   WideString* sError);

  bool get_red(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_red(CJS_Runtime* pRuntime, const CJS_Value& vp, WideString* sError);

  bool get_transparent(CJS_Runtime* pRuntime,
                       CJS_Value* vp,
                       WideString* sError);
  bool set_transparent(CJS_Runtime* pRuntime,
                       const CJS_Value& vp,
                       WideString* sError);

  bool get_white(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_white(CJS_Runtime* pRuntime,
                 const CJS_Value& vp,
                 WideString* sError);

  bool get_yellow(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_yellow(CJS_Runtime* pRuntime,
                  const CJS_Value& vp,
                  WideString* sError);

  bool convert(CJS_Runtime* pRuntime,
               const std::vector<CJS_Value>& params,
               CJS_Value& vRet,
               WideString& sError);
  bool equal(CJS_Runtime* pRuntime,
             const std::vector<CJS_Value>& params,
             CJS_Value& vRet,
             WideString& sError);

  static void ConvertPWLColorToArray(CJS_Runtime* pRuntime,
                                     const CFX_Color& color,
                                     CJS_Array* array);
  static void ConvertArrayToPWLColor(CJS_Runtime* pRuntime,
                                     const CJS_Array& array,
                                     CFX_Color* color);

 private:
  bool GetPropertyHelper(CJS_Runtime* pRuntime, CJS_Value* vp, CFX_Color* val);
  bool SetPropertyHelper(CJS_Runtime* pRuntime,
                         const CJS_Value& vp,
                         CFX_Color* val);

  CFX_Color m_crTransparent;
  CFX_Color m_crBlack;
  CFX_Color m_crWhite;
  CFX_Color m_crRed;
  CFX_Color m_crGreen;
  CFX_Color m_crBlue;
  CFX_Color m_crCyan;
  CFX_Color m_crMagenta;
  CFX_Color m_crYellow;
  CFX_Color m_crDKGray;
  CFX_Color m_crGray;
  CFX_Color m_crLTGray;
};

class CJS_Color : public CJS_Object {
 public:
  explicit CJS_Color(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Color() override {}

  DECLARE_JS_CLASS();

  JS_STATIC_PROP(black, black, color);
  JS_STATIC_PROP(blue, blue, color);
  JS_STATIC_PROP(cyan, cyan, color);
  JS_STATIC_PROP(dkGray, dark_gray, color);
  JS_STATIC_PROP(gray, gray, color);
  JS_STATIC_PROP(green, green, color);
  JS_STATIC_PROP(ltGray, light_gray, color);
  JS_STATIC_PROP(magenta, magenta, color);
  JS_STATIC_PROP(red, red, color);
  JS_STATIC_PROP(transparent, transparent, color);
  JS_STATIC_PROP(white, white, color);
  JS_STATIC_PROP(yellow, yellow, color);

  JS_STATIC_METHOD(convert, color);
  JS_STATIC_METHOD(equal, color);
};

#endif  // FPDFSDK_JAVASCRIPT_COLOR_H_
