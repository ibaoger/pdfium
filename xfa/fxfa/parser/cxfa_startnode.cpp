// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_startnode.h"

// static
bool CXFA_StartNode::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_StartNode::CXFA_StartNode(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::StartNode,
                L"startNode") {}

CXFA_StartNode::~CXFA_StartNode() {}

WideStringView CXFA_StartNode::GetName() const {
  return L"startNode";
}

XFA_Element CXFA_StartNode::GetElementType() const {
  return XFA_Element::StartNode;
}
