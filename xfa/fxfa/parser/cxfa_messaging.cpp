// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_messaging.h"

// static
bool CXFA_Messaging::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Messaging::CXFA_Messaging(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Messaging,
                L"messaging") {}

CXFA_Messaging::~CXFA_Messaging() {}

WideStringView CXFA_Messaging::GetName() const {
  return L"messaging";
}

XFA_Element CXFA_Messaging::GetElementType() const {
  return XFA_Element::Messaging;
}
