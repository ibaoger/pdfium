// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_ps.h"

// static
bool CXFA_Ps::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Ps::CXFA_Ps(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Ps, L"ps") {}

CXFA_Ps::~CXFA_Ps() {}

WideStringView CXFA_Ps::GetName() const {
  return L"ps";
}

XFA_Element CXFA_Ps::GetElementType() const {
  return XFA_Element::Ps;
}
