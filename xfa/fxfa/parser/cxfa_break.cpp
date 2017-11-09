// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_break.h"

// static
bool CXFA_Break::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Break::CXFA_Break(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Break,
                L"break") {}

CXFA_Break::~CXFA_Break() {}

WideStringView CXFA_Break::GetName() const {
  return L"break";
}

XFA_Element CXFA_Break::GetElementType() const {
  return XFA_Element::Break;
}
