// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_keep.h"

// static
bool CXFA_Keep::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Keep::CXFA_Keep(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Keep, L"keep") {
}

CXFA_Keep::~CXFA_Keep() {}

WideStringView CXFA_Keep::GetName() const {
  return L"keep";
}

XFA_Element CXFA_Keep::GetElementType() const {
  return XFA_Element::Keep;
}
