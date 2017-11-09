// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_variables.h"

// static
bool CXFA_Variables::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Variables::CXFA_Variables(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContainerNode,
                XFA_Element::Variables,
                L"variables") {}

CXFA_Variables::~CXFA_Variables() {}

WideStringView CXFA_Variables::GetName() const {
  return L"variables";
}

XFA_Element CXFA_Variables::GetElementType() const {
  return XFA_Element::Variables;
}
