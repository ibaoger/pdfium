// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_mdp.h"

// static
bool CXFA_Mdp::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Mdp::CXFA_Mdp(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Mdp, L"mdp") {}

CXFA_Mdp::~CXFA_Mdp() {}

WideStringView CXFA_Mdp::GetName() const {
  return L"mdp";
}

XFA_Element CXFA_Mdp::GetElementType() const {
  return XFA_Element::Mdp;
}
