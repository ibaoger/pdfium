// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_openaction.h"

// static
bool CXFA_OpenAction::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_OpenAction::CXFA_OpenAction(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::OpenAction,
                L"openAction") {}

CXFA_OpenAction::~CXFA_OpenAction() {}

WideStringView CXFA_OpenAction::GetName() const {
  return L"openAction";
}

XFA_Element CXFA_OpenAction::GetElementType() const {
  return XFA_Element::OpenAction;
}
