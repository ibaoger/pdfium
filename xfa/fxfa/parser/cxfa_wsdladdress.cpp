// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_wsdladdress.h"

// static
bool CXFA_WsdlAddress::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_ConnectionSet & packet);
}

CXFA_WsdlAddress::CXFA_WsdlAddress(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::WsdlAddress,
                L"wsdlAddress") {}

CXFA_WsdlAddress::~CXFA_WsdlAddress() {}

WideStringView CXFA_WsdlAddress::GetName() const {
  return L"wsdlAddress";
}

XFA_Element CXFA_WsdlAddress::GetElementType() const {
  return XFA_Element::WsdlAddress;
}
