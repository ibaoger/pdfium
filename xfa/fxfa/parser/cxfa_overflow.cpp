// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_overflow.h"

// static
bool CXFA_Overflow::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Overflow::CXFA_Overflow(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Overflow,
                L"overflow") {}

CXFA_Overflow::~CXFA_Overflow() {}

WideStringView CXFA_Overflow::GetName() const {
  return L"overflow";
}

XFA_Element CXFA_Overflow::GetElementType() const {
  return XFA_Element::Overflow;
}
