// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_adbe_jsdebugger.h"

// static
bool CXFA_aDBE_JSDebugger::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_aDBE_JSDebugger::CXFA_aDBE_JSDebugger(CXFA_Document* doc,
                                           XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::ADBE_JSDebugger,
                L"ADBE_JSDebugger") {}

CXFA_aDBE_JSDebugger::~CXFA_aDBE_JSDebugger() {}

WideStringView CXFA_aDBE_JSDebugger::GetName() const {
  return L"ADBE_JSDebugger";
}

XFA_Element CXFA_aDBE_JSDebugger::GetElementType() const {
  return XFA_Element::ADBE_JSDebugger;
}
