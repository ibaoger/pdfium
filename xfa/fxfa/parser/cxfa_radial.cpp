// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_radial.h"

// static
bool CXFA_Radial::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Radial::CXFA_Radial(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Radial,
                L"radial") {}

CXFA_Radial::~CXFA_Radial() {}

WideStringView CXFA_Radial::GetName() const {
  return L"radial";
}

XFA_Element CXFA_Radial::GetElementType() const {
  return XFA_Element::Radial;
}
