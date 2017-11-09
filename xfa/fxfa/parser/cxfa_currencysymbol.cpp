// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_currencysymbol.h"

// static
bool CXFA_CurrencySymbol::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_CurrencySymbol::CXFA_CurrencySymbol(CXFA_Document* doc,
                                         XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::CurrencySymbol,
                L"currencySymbol") {}

CXFA_CurrencySymbol::~CXFA_CurrencySymbol() {}

WideStringView CXFA_CurrencySymbol::GetName() const {
  return L"currencySymbol";
}

XFA_Element CXFA_CurrencySymbol::GetElementType() const {
  return XFA_Element::CurrencySymbol;
}
