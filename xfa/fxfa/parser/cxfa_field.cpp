// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_field.h"

// static
bool CXFA_Field::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Field::CXFA_Field(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContainerNode,
                XFA_Element::Field,
                L"field") {}

CXFA_Field::~CXFA_Field() {}

WideStringView CXFA_Field::GetName() const {
  return L"field";
}

XFA_Element CXFA_Field::GetElementType() const {
  return XFA_Element::Field;
}
