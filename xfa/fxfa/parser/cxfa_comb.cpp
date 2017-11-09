// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_comb.h"

// static
bool CXFA_Comb::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Comb::CXFA_Comb(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Comb, L"comb") {
}

CXFA_Comb::~CXFA_Comb() {}

WideStringView CXFA_Comb::GetName() const {
  return L"comb";
}

XFA_Element CXFA_Comb::GetElementType() const {
  return XFA_Element::Comb;
}
