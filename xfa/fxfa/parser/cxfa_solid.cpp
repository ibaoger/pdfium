// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_solid.h"

// static
bool CXFA_Solid::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Solid::CXFA_Solid(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Solid,
                L"solid") {}

CXFA_Solid::~CXFA_Solid() {}

WideStringView CXFA_Solid::GetName() const {
  return L"solid";
}

XFA_Element CXFA_Solid::GetElementType() const {
  return XFA_Element::Solid;
}
