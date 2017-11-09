// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_tooltip.h"

// static
bool CXFA_ToolTip::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_ToolTip::CXFA_ToolTip(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::ToolTip,
                L"toolTip") {}

CXFA_ToolTip::~CXFA_ToolTip() {}

WideStringView CXFA_ToolTip::GetName() const {
  return L"toolTip";
}

XFA_Element CXFA_ToolTip::GetElementType() const {
  return XFA_Element::ToolTip;
}
