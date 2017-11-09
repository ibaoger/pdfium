// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_corner.h"

// static
bool CXFA_Corner::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Corner::CXFA_Corner(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Corner,
                L"corner") {}

CXFA_Corner::~CXFA_Corner() {}

WideStringView CXFA_Corner::GetName() const {
  return L"corner";
}

XFA_Element CXFA_Corner::GetElementType() const {
  return XFA_Element::Corner;
}
