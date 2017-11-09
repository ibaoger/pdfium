// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_type.h"

// static
bool CXFA_Type::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Type::CXFA_Type(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Type,
                L"type") {}

CXFA_Type::~CXFA_Type() {}

WideStringView CXFA_Type::GetName() const {
  return L"type";
}

XFA_Element CXFA_Type::GetElementType() const {
  return XFA_Element::Type;
}
