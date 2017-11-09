// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_binditems.h"

// static
bool CXFA_BindItems::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_BindItems::CXFA_BindItems(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::BindItems,
                L"bindItems") {}

CXFA_BindItems::~CXFA_BindItems() {}

WideStringView CXFA_BindItems::GetName() const {
  return L"bindItems";
}

XFA_Element CXFA_BindItems::GetElementType() const {
  return XFA_Element::BindItems;
}
