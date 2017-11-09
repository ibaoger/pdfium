// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_whitespace.h"

// static
bool CXFA_Whitespace::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Whitespace::CXFA_Whitespace(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::Whitespace,
                L"whitespace") {}

CXFA_Whitespace::~CXFA_Whitespace() {}

WideStringView CXFA_Whitespace::GetName() const {
  return L"whitespace";
}

XFA_Element CXFA_Whitespace::GetElementType() const {
  return XFA_Element::Whitespace;
}
