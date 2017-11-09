// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_items.h"

// static
bool CXFA_Items::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Items::CXFA_Items(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Items,
                L"items") {}

CXFA_Items::~CXFA_Items() {}

WideStringView CXFA_Items::GetName() const {
  return L"items";
}

XFA_Element CXFA_Items::GetElementType() const {
  return XFA_Element::Items;
}
