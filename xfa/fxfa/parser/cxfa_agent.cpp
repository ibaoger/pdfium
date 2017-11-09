// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_agent.h"

// static
bool CXFA_Agent::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Agent::CXFA_Agent(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Agent,
                L"agent") {}

CXFA_Agent::~CXFA_Agent() {}

WideStringView CXFA_Agent::GetName() const {
  return L"agent";
}

XFA_Element CXFA_Agent::GetElementType() const {
  return XFA_Element::Agent;
}
