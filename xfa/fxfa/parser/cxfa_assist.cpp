// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_assist.h"

// static
bool CXFA_Assist::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Assist::CXFA_Assist(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Assist,
                L"assist") {}

CXFA_Assist::~CXFA_Assist() {}

WideStringView CXFA_Assist::GetName() const {
  return L"assist";
}

XFA_Element CXFA_Assist::GetElementType() const {
  return XFA_Element::Assist;
}
