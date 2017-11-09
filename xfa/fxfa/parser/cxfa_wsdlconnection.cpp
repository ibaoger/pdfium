// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_wsdlconnection.h"

// static
bool CXFA_WsdlConnection::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_ConnectionSet & packet);
}

CXFA_WsdlConnection::CXFA_WsdlConnection(CXFA_Document* doc,
                                         XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::WsdlConnection,
                L"wsdlConnection") {}

CXFA_WsdlConnection::~CXFA_WsdlConnection() {}

WideStringView CXFA_WsdlConnection::GetName() const {
  return L"wsdlConnection";
}

XFA_Element CXFA_WsdlConnection::GetElementType() const {
  return XFA_Element::WsdlConnection;
}
