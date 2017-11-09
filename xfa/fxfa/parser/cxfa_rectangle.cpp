// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_rectangle.h"

// static
bool CXFA_Rectangle::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Rectangle::CXFA_Rectangle(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Rectangle,
                L"rectangle") {}

CXFA_Rectangle::~CXFA_Rectangle() {}

WideStringView CXFA_Rectangle::GetName() const {
  return L"rectangle";
}

XFA_Element CXFA_Rectangle::GetElementType() const {
  return XFA_Element::Rectangle;
}
