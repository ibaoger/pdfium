// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_jog.h"

// static
bool CXFA_Jog::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Jog::CXFA_Jog(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Jog,
                L"jog") {}

CXFA_Jog::~CXFA_Jog() {}

WideStringView CXFA_Jog::GetName() const {
  return L"jog";
}

XFA_Element CXFA_Jog::GetElementType() const {
  return XFA_Element::Jog;
}
