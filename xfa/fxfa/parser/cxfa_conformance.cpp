// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_conformance.h"

// static
bool CXFA_Conformance::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Conformance::CXFA_Conformance(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Conformance,
                L"conformance") {}

CXFA_Conformance::~CXFA_Conformance() {}

WideStringView CXFA_Conformance::GetName() const {
  return L"conformance";
}

XFA_Element CXFA_Conformance::GetElementType() const {
  return XFA_Element::Conformance;
}
