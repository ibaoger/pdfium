// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_certificates.h"

// static
bool CXFA_Certificates::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Certificates::CXFA_Certificates(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Certificates,
                L"certificates") {}

CXFA_Certificates::~CXFA_Certificates() {}

WideStringView CXFA_Certificates::GetName() const {
  return L"certificates";
}

XFA_Element CXFA_Certificates::GetElementType() const {
  return XFA_Element::Certificates;
}
