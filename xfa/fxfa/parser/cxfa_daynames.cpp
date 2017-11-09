// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_daynames.h"

// static
bool CXFA_DayNames::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_DayNames::CXFA_DayNames(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::DayNames,
                L"dayNames") {}

CXFA_DayNames::~CXFA_DayNames() {}

WideStringView CXFA_DayNames::GetName() const {
  return L"dayNames";
}

XFA_Element CXFA_DayNames::GetElementType() const {
  return XFA_Element::DayNames;
}
