// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_numbersymbol.h"

// static
bool CXFA_NumberSymbol::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_NumberSymbol::CXFA_NumberSymbol(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::NumberSymbol,
                L"numberSymbol") {}

CXFA_NumberSymbol::~CXFA_NumberSymbol() {}

WideStringView CXFA_NumberSymbol::GetName() const {
  return L"numberSymbol";
}

XFA_Element CXFA_NumberSymbol::GetElementType() const {
  return XFA_Element::NumberSymbol;
}
