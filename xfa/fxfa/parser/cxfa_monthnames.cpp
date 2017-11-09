// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_monthnames.h"

// static
bool CXFA_MonthNames::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_MonthNames::CXFA_MonthNames(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::MonthNames,
                L"monthNames") {}

CXFA_MonthNames::~CXFA_MonthNames() {}

WideStringView CXFA_MonthNames::GetName() const {
  return L"monthNames";
}

XFA_Element CXFA_MonthNames::GetElementType() const {
  return XFA_Element::MonthNames;
}
