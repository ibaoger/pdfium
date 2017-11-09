// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_relevant.h"

// static
bool CXFA_Relevant::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Relevant::CXFA_Relevant(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::Relevant,
                L"relevant") {}

CXFA_Relevant::~CXFA_Relevant() {}

WideStringView CXFA_Relevant::GetName() const {
  return L"relevant";
}

XFA_Element CXFA_Relevant::GetElementType() const {
  return XFA_Element::Relevant;
}
