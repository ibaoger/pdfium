// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_to.h"

// static
bool CXFA_To::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_To::CXFA_To(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::To,
                L"to") {}

CXFA_To::~CXFA_To() {}

WideStringView CXFA_To::GetName() const {
  return L"to";
}

XFA_Element CXFA_To::GetElementType() const {
  return XFA_Element::To;
}
