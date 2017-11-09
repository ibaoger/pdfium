// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_color.h"

// static
bool CXFA_Color::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Color::CXFA_Color(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Color,
                L"color") {}

CXFA_Color::~CXFA_Color() {}

WideStringView CXFA_Color::GetName() const {
  return L"color";
}

XFA_Element CXFA_Color::GetElementType() const {
  return XFA_Element::Color;
}
