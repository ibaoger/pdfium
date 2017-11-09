// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_soapaddress.h"

// static
bool CXFA_SoapAddress::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_ConnectionSet & packet);
}

CXFA_SoapAddress::CXFA_SoapAddress(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::SoapAddress,
                L"soapAddress") {}

CXFA_SoapAddress::~CXFA_SoapAddress() {}

WideStringView CXFA_SoapAddress::GetName() const {
  return L"soapAddress";
}

XFA_Element CXFA_SoapAddress::GetElementType() const {
  return XFA_Element::SoapAddress;
}
