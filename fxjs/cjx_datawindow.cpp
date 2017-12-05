// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjx_datawindow.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cscript_datawindow.h"

const CJX_MethodSpec CJX_DataWindow::MethodSpecs[] = {
    {"gotoRecord", gotoRecord_static},
    {"isRecordGroup", isRecordGroup_static},
    {"moveCurrentRecord", moveCurrentRecord_static},
    {"record", record_static},
    {"", nullptr}};

CJX_DataWindow::CJX_DataWindow(CScript_DataWindow* window)
    : CJX_Object(window) {}

CJX_DataWindow::~CJX_DataWindow() {}

bool CJX_DataWindow::HasMethod(const WideString& func) const {
  if (HasMethodFromSpecs(MethodSpecs, func))
    return true;
  return CJX_Object::HasMethod(func);
}

void CJX_DataWindow::RunMethod(const WideString& func, CFXJSE_Arguments* args) {
  if (RunMethodFromSpecs(MethodSpecs, func, args))
    return;
  CJX_Object::RunMethod(func, args);
}

void CJX_DataWindow::moveCurrentRecord(CFXJSE_Arguments* pArguments) {}

void CJX_DataWindow::record(CFXJSE_Arguments* pArguments) {}

void CJX_DataWindow::gotoRecord(CFXJSE_Arguments* pArguments) {}

void CJX_DataWindow::isRecordGroup(CFXJSE_Arguments* pArguments) {}

void CJX_DataWindow::RecordsBefore(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {}

void CJX_DataWindow::CurrentRecordNumber(CFXJSE_Value* pValue,
                                         bool bSetting,
                                         XFA_Attribute eAttribute) {}

void CJX_DataWindow::RecordsAfter(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute) {}

void CJX_DataWindow::IsDefined(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute) {}
