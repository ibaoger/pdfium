// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_datetimesymbols.h"

// static
bool CXFA_DateTimeSymbols::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_DateTimeSymbols::CXFA_DateTimeSymbols(CXFA_Document* doc,
                                           XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::DateTimeSymbols,
                L"dateTimeSymbols") {}

CXFA_DateTimeSymbols::~CXFA_DateTimeSymbols() {}

WideStringView CXFA_DateTimeSymbols::GetName() const {
  return L"dateTimeSymbols";
}

XFA_Element CXFA_DateTimeSymbols::GetElementType() const {
  return XFA_Element::DateTimeSymbols;
}
