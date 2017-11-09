// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_copies.h"

// static
bool CXFA_Copies::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Copies::CXFA_Copies(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Copies,
                L"copies") {}

CXFA_Copies::~CXFA_Copies() {}

WideStringView CXFA_Copies::GetName() const {
  return L"copies";
}

XFA_Element CXFA_Copies::GetElementType() const {
  return XFA_Element::Copies;
}
