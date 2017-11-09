// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_rootelement.h"

// static
bool CXFA_RootElement::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_ConnectionSet & packet);
}

CXFA_RootElement::CXFA_RootElement(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::RootElement,
                L"rootElement") {}

CXFA_RootElement::~CXFA_RootElement() {}

WideStringView CXFA_RootElement::GetName() const {
  return L"rootElement";
}

XFA_Element CXFA_RootElement::GetElementType() const {
  return XFA_Element::RootElement;
}
