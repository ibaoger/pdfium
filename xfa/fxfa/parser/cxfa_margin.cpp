// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_margin.h"

// static
bool CXFA_Margin::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Margin::CXFA_Margin(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Margin,
                L"margin") {}

CXFA_Margin::~CXFA_Margin() {}

WideStringView CXFA_Margin::GetName() const {
  return L"margin";
}

XFA_Element CXFA_Margin::GetElementType() const {
  return XFA_Element::Margin;
}
