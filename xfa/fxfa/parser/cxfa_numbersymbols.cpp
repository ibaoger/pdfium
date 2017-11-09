// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_numbersymbols.h"

// static
bool CXFA_NumberSymbols::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_NumberSymbols::CXFA_NumberSymbols(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::NumberSymbols,
                L"numberSymbols") {}

CXFA_NumberSymbols::~CXFA_NumberSymbols() {}

WideStringView CXFA_NumberSymbols::GetName() const {
  return L"numberSymbols";
}

XFA_Element CXFA_NumberSymbols::GetElementType() const {
  return XFA_Element::NumberSymbols;
}
