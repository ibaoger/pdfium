// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_attributes.h"

// static
bool CXFA_Attributes::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Attributes::CXFA_Attributes(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::Attributes,
                L"attributes") {}

CXFA_Attributes::~CXFA_Attributes() {}

WideStringView CXFA_Attributes::GetName() const {
  return L"attributes";
}

XFA_Element CXFA_Attributes::GetElementType() const {
  return XFA_Element::Attributes;
}
