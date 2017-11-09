// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_border.h"

// static
bool CXFA_Border::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Border::CXFA_Border(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Border,
                L"border") {}

CXFA_Border::~CXFA_Border() {}

WideStringView CXFA_Border::GetName() const {
  return L"border";
}

XFA_Element CXFA_Border::GetElementType() const {
  return XFA_Element::Border;
}
