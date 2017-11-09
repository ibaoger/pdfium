// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pdfa.h"

// static
bool CXFA_Pdfa::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Pdfa::CXFA_Pdfa(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Pdfa, L"pdfa") {
}

CXFA_Pdfa::~CXFA_Pdfa() {}

WideStringView CXFA_Pdfa::GetName() const {
  return L"pdfa";
}

XFA_Element CXFA_Pdfa::GetElementType() const {
  return XFA_Element::Pdfa;
}
