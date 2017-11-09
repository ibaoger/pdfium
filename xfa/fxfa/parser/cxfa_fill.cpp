// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_fill.h"

// static
bool CXFA_Fill::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Fill::CXFA_Fill(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Fill, L"fill") {
}

CXFA_Fill::~CXFA_Fill() {}

WideStringView CXFA_Fill::GetName() const {
  return L"fill";
}

XFA_Element CXFA_Fill::GetElementType() const {
  return XFA_Element::Fill;
}
