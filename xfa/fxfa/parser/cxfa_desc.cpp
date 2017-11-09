// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_desc.h"

// static
bool CXFA_Desc::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Desc::CXFA_Desc(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Desc, L"desc") {
}

CXFA_Desc::~CXFA_Desc() {}

WideStringView CXFA_Desc::GetName() const {
  return L"desc";
}

XFA_Element CXFA_Desc::GetElementType() const {
  return XFA_Element::Desc;
}
