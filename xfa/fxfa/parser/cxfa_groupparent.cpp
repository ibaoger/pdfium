// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_groupparent.h"

// static
bool CXFA_GroupParent::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_GroupParent::CXFA_GroupParent(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::GroupParent,
                L"groupParent") {}

CXFA_GroupParent::~CXFA_GroupParent() {}

WideStringView CXFA_GroupParent::GetName() const {
  return L"groupParent";
}

XFA_Element CXFA_GroupParent::GetElementType() const {
  return XFA_Element::GroupParent;
}
