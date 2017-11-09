// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_edge.h"

// static
bool CXFA_Edge::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Edge::CXFA_Edge(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Edge, L"edge") {
}

CXFA_Edge::~CXFA_Edge() {}

WideStringView CXFA_Edge::GetName() const {
  return L"edge";
}

XFA_Element CXFA_Edge::GetElementType() const {
  return XFA_Element::Edge;
}
