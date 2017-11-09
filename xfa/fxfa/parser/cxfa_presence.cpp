// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_presence.h"

// static
bool CXFA_Presence::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Presence::CXFA_Presence(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::Presence,
                L"presence") {}

CXFA_Presence::~CXFA_Presence() {}

WideStringView CXFA_Presence::GetName() const {
  return L"presence";
}

XFA_Element CXFA_Presence::GetElementType() const {
  return XFA_Element::Presence;
}
