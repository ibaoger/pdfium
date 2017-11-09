// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_datepatterns.h"

// static
bool CXFA_DatePatterns::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_DatePatterns::CXFA_DatePatterns(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::DatePatterns,
                L"datePatterns") {}

CXFA_DatePatterns::~CXFA_DatePatterns() {}

WideStringView CXFA_DatePatterns::GetName() const {
  return L"datePatterns";
}

XFA_Element CXFA_DatePatterns::GetElementType() const {
  return XFA_Element::DatePatterns;
}
