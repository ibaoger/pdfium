// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_select.h"

// static
bool CXFA_Select::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_SourceSet & packet);
}

CXFA_Select::CXFA_Select(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::Select,
                L"select") {}

CXFA_Select::~CXFA_Select() {}

WideStringView CXFA_Select::GetName() const {
  return L"select";
}

XFA_Element CXFA_Select::GetElementType() const {
  return XFA_Element::Select;
}
