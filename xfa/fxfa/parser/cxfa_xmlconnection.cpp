// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_xmlconnection.h"

// static
bool CXFA_XmlConnection::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_ConnectionSet & packet);
}

CXFA_XmlConnection::CXFA_XmlConnection(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::XmlConnection,
                L"xmlConnection") {}

CXFA_XmlConnection::~CXFA_XmlConnection() {}

WideStringView CXFA_XmlConnection::GetName() const {
  return L"xmlConnection";
}

XFA_Element CXFA_XmlConnection::GetElementType() const {
  return XFA_Element::XmlConnection;
}
