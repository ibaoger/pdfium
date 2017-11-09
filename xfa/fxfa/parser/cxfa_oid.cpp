// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_oid.h"

// static
bool CXFA_Oid::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Oid::CXFA_Oid(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::Oid,
                L"oid") {}

CXFA_Oid::~CXFA_Oid() {}

WideStringView CXFA_Oid::GetName() const {
  return L"oid";
}

XFA_Element CXFA_Oid::GetElementType() const {
  return XFA_Element::Oid;
}
