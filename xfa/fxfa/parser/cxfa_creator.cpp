// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_creator.h"

// static
bool CXFA_Creator::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Creator::CXFA_Creator(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Creator,
                L"creator") {}

CXFA_Creator::~CXFA_Creator() {}

WideStringView CXFA_Creator::GetName() const {
  return L"creator";
}

XFA_Element CXFA_Creator::GetElementType() const {
  return XFA_Element::Creator;
}
