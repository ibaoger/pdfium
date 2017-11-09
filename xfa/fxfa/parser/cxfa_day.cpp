// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_day.h"

// static
bool CXFA_Day::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_Day::CXFA_Day(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Day,
                L"day") {}

CXFA_Day::~CXFA_Day() {}

WideStringView CXFA_Day::GetName() const {
  return L"day";
}

XFA_Element CXFA_Day::GetElementType() const {
  return XFA_Element::Day;
}
