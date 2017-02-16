// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/event.h"

#include "fpdfsdk/javascript/Field.h"
#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_EventHandler.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/cjs_context.h"

JSConstSpec CJS_Event::ConstSpecs[] = {{0, JSConstSpec::Number, 0, 0}};

JSPropertySpec CJS_Event::PropertySpecs[] = {
    {L"change", get_change_static, set_change_static},
    {L"changeEx", get_changeEx_static, set_changeEx_static},
    {L"commitKey", get_commitKey_static, set_commitKey_static},
    {L"fieldFull", get_fieldFull_static, set_fieldFull_static},
    {L"keyDown", get_keyDown_static, set_keyDown_static},
    {L"modifier", get_modifier_static, set_modifier_static},
    {L"name", get_name_static, set_name_static},
    {L"rc", get_rc_static, set_rc_static},
    {L"richChange", get_richChange_static, set_richChange_static},
    {L"richChangeEx", get_richChangeEx_static, set_richChangeEx_static},
    {L"richValue", get_richValue_static, set_richValue_static},
    {L"selEnd", get_selEnd_static, set_selEnd_static},
    {L"selStart", get_selStart_static, set_selStart_static},
    {L"shift", get_shift_static, set_shift_static},
    {L"source", get_source_static, set_source_static},
    {L"target", get_target_static, set_target_static},
    {L"targetName", get_targetName_static, set_targetName_static},
    {L"type", get_type_static, set_type_static},
    {L"value", get_value_static, set_value_static},
    {L"willCommit", get_willCommit_static, set_willCommit_static},
    {0, 0, 0}};

JSMethodSpec CJS_Event::MethodSpecs[] = {{0, 0}};

IMPLEMENT_JS_CLASS(CJS_Event, event)

event::event(CJS_Object* pJsObject) : CJS_EmbedObj(pJsObject) {}

event::~event() {}

bool event::change(IJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError) {
  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();
  CFX_WideString& wChange = pEvent->Change();
  if (vp.IsSetting()) {
    if (vp.GetJSValue()->GetType() == CJS_Value::VT_string)
      vp >> wChange;
  } else {
    vp << wChange;
  }
  return true;
}

bool event::changeEx(IJS_Context* cc,
                     CJS_PropValue& vp,
                     CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  vp << pEvent->ChangeEx();
  return true;
}

bool event::commitKey(IJS_Context* cc,
                      CJS_PropValue& vp,
                      CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  vp << pEvent->CommitKey();
  return true;
}

bool event::fieldFull(IJS_Context* cc,
                      CJS_PropValue& vp,
                      CFX_WideString& sError) {
  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  if (!vp.IsGetting() &&
      wcscmp((const wchar_t*)pEvent->Name(), L"Keystroke") != 0)
    return false;

  if (pEvent->FieldFull())
    vp << true;
  else
    vp << false;
  return true;
}

bool event::keyDown(IJS_Context* cc,
                    CJS_PropValue& vp,
                    CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  if (pEvent->KeyDown())
    vp << true;
  else
    vp << false;
  return true;
}

bool event::modifier(IJS_Context* cc,
                     CJS_PropValue& vp,
                     CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  if (pEvent->Modifier())
    vp << true;
  else
    vp << false;
  return true;
}

bool event::name(IJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  vp << pEvent->Name();
  return true;
}

bool event::rc(IJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError) {
  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  bool& bRc = pEvent->Rc();
  if (vp.IsSetting()) {
    vp >> bRc;
  } else {
    vp << bRc;
  }
  return true;
}

bool event::richChange(IJS_Context* cc,
                       CJS_PropValue& vp,
                       CFX_WideString& sError) {
  return true;
}

bool event::richChangeEx(IJS_Context* cc,
                         CJS_PropValue& vp,
                         CFX_WideString& sError) {
  return true;
}

bool event::richValue(IJS_Context* cc,
                      CJS_PropValue& vp,
                      CFX_WideString& sError) {
  return true;
}

bool event::selEnd(IJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError) {
  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  if (wcscmp((const wchar_t*)pEvent->Name(), L"Keystroke") != 0) {
    return true;
  }

  int& iSelEnd = pEvent->SelEnd();
  if (vp.IsSetting()) {
    vp >> iSelEnd;
  } else {
    vp << iSelEnd;
  }
  return true;
}

bool event::selStart(IJS_Context* cc,
                     CJS_PropValue& vp,
                     CFX_WideString& sError) {
  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  if (wcscmp((const wchar_t*)pEvent->Name(), L"Keystroke") != 0) {
    return true;
  }
  int& iSelStart = pEvent->SelStart();
  if (vp.IsSetting()) {
    vp >> iSelStart;
  } else {
    vp << iSelStart;
  }
  return true;
}

bool event::shift(IJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  if (pEvent->Shift())
    vp << true;
  else
    vp << false;
  return true;
}

bool event::source(IJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  vp << pEvent->Source()->GetJSObject();
  return true;
}

bool event::target(IJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  vp << pEvent->Target_Field()->GetJSObject();
  return true;
}

bool event::targetName(IJS_Context* cc,
                       CJS_PropValue& vp,
                       CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  vp << pEvent->TargetName();
  return true;
}

bool event::type(IJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  vp << pEvent->Type();
  return true;
}

bool event::value(IJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError) {
  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  if (wcscmp((const wchar_t*)pEvent->Type(), L"Field") != 0)
    return false;
  if (!pEvent->m_pValue)
    return false;
  CFX_WideString& val = pEvent->Value();
  if (vp.IsSetting()) {
    vp >> val;
  } else {
    vp << val;
  }
  return true;
}

bool event::willCommit(IJS_Context* cc,
                       CJS_PropValue& vp,
                       CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  if (pEvent->WillCommit())
    vp << true;
  else
    vp << false;
  return true;
}
