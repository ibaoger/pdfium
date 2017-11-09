// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_adbe_jsconsole.h"

// static
bool CXFA_aDBE_JSConsole::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_aDBE_JSConsole::CXFA_aDBE_JSConsole(CXFA_Document* doc,
                                         XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::ADBE_JSConsole,
                L"ADBE_JSConsole") {}

CXFA_aDBE_JSConsole::~CXFA_aDBE_JSConsole() {}

WideStringView CXFA_aDBE_JSConsole::GetName() const {
  return L"ADBE_JSConsole";
}

XFA_Element CXFA_aDBE_JSConsole::GetElementType() const {
  return XFA_Element::ADBE_JSConsole;
}
