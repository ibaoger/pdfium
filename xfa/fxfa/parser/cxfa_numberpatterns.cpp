// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_numberpatterns.h"

// static
bool CXFA_NumberPatterns::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_NumberPatterns::CXFA_NumberPatterns(CXFA_Document* doc,
                                         XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::NumberPatterns,
                L"numberPatterns") {}

CXFA_NumberPatterns::~CXFA_NumberPatterns() {}

WideStringView CXFA_NumberPatterns::GetName() const {
  return L"numberPatterns";
}

XFA_Element CXFA_NumberPatterns::GetElementType() const {
  return XFA_Element::NumberPatterns;
}
