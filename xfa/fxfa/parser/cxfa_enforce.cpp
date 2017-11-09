// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_enforce.h"

// static
bool CXFA_Enforce::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Enforce::CXFA_Enforce(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Enforce,
                L"enforce") {}

CXFA_Enforce::~CXFA_Enforce() {}

WideStringView CXFA_Enforce::GetName() const {
  return L"enforce";
}

XFA_Element CXFA_Enforce::GetElementType() const {
  return XFA_Element::Enforce;
}
