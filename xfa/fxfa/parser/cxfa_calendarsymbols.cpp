// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_calendarsymbols.h"

// static
bool CXFA_CalendarSymbols::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_CalendarSymbols::CXFA_CalendarSymbols(CXFA_Document* doc,
                                           XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::CalendarSymbols,
                L"calendarSymbols") {}

CXFA_CalendarSymbols::~CXFA_CalendarSymbols() {}

WideStringView CXFA_CalendarSymbols::GetName() const {
  return L"calendarSymbols";
}

XFA_Element CXFA_CalendarSymbols::GetElementType() const {
  return XFA_Element::CalendarSymbols;
}
