// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_interactive.h"

// static
bool CXFA_Interactive::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Interactive::CXFA_Interactive(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Interactive,
                L"interactive") {}

CXFA_Interactive::~CXFA_Interactive() {}

WideStringView CXFA_Interactive::GetName() const {
  return L"interactive";
}

XFA_Element CXFA_Interactive::GetElementType() const {
  return XFA_Element::Interactive;
}
