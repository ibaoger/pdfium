// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_meridiem.h"

// static
bool CXFA_Meridiem::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_Meridiem::CXFA_Meridiem(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Meridiem,
                L"meridiem") {}

CXFA_Meridiem::~CXFA_Meridiem() {}

WideStringView CXFA_Meridiem::GetName() const {
  return L"meridiem";
}

XFA_Element CXFA_Meridiem::GetElementType() const {
  return XFA_Element::Meridiem;
}
