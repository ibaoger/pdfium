// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_currencysymbols.h"

// static
bool CXFA_CurrencySymbols::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_CurrencySymbols::CXFA_CurrencySymbols(CXFA_Document* doc,
                                           XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::CurrencySymbols,
                L"currencySymbols") {}

CXFA_CurrencySymbols::~CXFA_CurrencySymbols() {}

WideStringView CXFA_CurrencySymbols::GetName() const {
  return L"currencySymbols";
}

XFA_Element CXFA_CurrencySymbols::GetElementType() const {
  return XFA_Element::CurrencySymbols;
}
