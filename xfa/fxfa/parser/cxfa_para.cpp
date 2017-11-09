// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_para.h"

// static
bool CXFA_Para::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Para::CXFA_Para(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Para, L"para") {
}

CXFA_Para::~CXFA_Para() {}

WideStringView CXFA_Para::GetName() const {
  return L"para";
}

XFA_Element CXFA_Para::GetElementType() const {
  return XFA_Element::Para;
}
