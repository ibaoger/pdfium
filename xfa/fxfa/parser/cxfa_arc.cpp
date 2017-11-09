// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_arc.h"

// static
bool CXFA_Arc::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Arc::CXFA_Arc(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Arc, L"arc") {}

CXFA_Arc::~CXFA_Arc() {}

WideStringView CXFA_Arc::GetName() const {
  return L"arc";
}

XFA_Element CXFA_Arc::GetElementType() const {
  return XFA_Element::Arc;
}
