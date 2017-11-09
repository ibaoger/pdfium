// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_ifempty.h"

// static
bool CXFA_IfEmpty::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_IfEmpty::CXFA_IfEmpty(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::IfEmpty,
                L"ifEmpty") {}

CXFA_IfEmpty::~CXFA_IfEmpty() {}

WideStringView CXFA_IfEmpty::GetName() const {
  return L"ifEmpty";
}

XFA_Element CXFA_IfEmpty::GetElementType() const {
  return XFA_Element::IfEmpty;
}
