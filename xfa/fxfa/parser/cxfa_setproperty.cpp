// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_setproperty.h"

// static
bool CXFA_SetProperty::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_SetProperty::CXFA_SetProperty(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::SetProperty,
                L"setProperty") {}

CXFA_SetProperty::~CXFA_SetProperty() {}

WideStringView CXFA_SetProperty::GetName() const {
  return L"setProperty";
}

XFA_Element CXFA_SetProperty::GetElementType() const {
  return XFA_Element::SetProperty;
}
