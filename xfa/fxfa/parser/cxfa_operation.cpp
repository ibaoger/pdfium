// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_operation.h"

// static
bool CXFA_Operation::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_ConnectionSet & packet);
}

CXFA_Operation::CXFA_Operation(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::Operation,
                L"operation") {}

CXFA_Operation::~CXFA_Operation() {}

WideStringView CXFA_Operation::GetName() const {
  return L"operation";
}

XFA_Element CXFA_Operation::GetElementType() const {
  return XFA_Element::Operation;
}
