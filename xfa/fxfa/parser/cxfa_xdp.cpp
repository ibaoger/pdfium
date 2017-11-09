// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_xdp.h"

// static
bool CXFA_Xdp::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Xdp::CXFA_Xdp(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Xdp, L"xdp") {}

CXFA_Xdp::~CXFA_Xdp() {}

WideStringView CXFA_Xdp::GetName() const {
  return L"xdp";
}

XFA_Element CXFA_Xdp::GetElementType() const {
  return XFA_Element::Xdp;
}
