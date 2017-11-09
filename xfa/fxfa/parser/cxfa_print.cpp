// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_print.h"

// static
bool CXFA_Print::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Print::CXFA_Print(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Print,
                L"print") {}

CXFA_Print::~CXFA_Print() {}

WideStringView CXFA_Print::GetName() const {
  return L"print";
}

XFA_Element CXFA_Print::GetElementType() const {
  return XFA_Element::Print;
}
