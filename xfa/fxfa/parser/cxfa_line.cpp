// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_line.h"

// static
bool CXFA_Line::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Line::CXFA_Line(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Line, L"line") {
}

CXFA_Line::~CXFA_Line() {}

WideStringView CXFA_Line::GetName() const {
  return L"line";
}

XFA_Element CXFA_Line::GetElementType() const {
  return XFA_Element::Line;
}
