// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_month.h"

// static
bool CXFA_Month::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_Month::CXFA_Month(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Month,
                L"month") {}

CXFA_Month::~CXFA_Month() {}

WideStringView CXFA_Month::GetName() const {
  return L"month";
}

XFA_Element CXFA_Month::GetElementType() const {
  return XFA_Element::Month;
}
