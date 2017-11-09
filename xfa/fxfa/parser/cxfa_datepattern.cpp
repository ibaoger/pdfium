// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_datepattern.h"

// static
bool CXFA_DatePattern::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_DatePattern::CXFA_DatePattern(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::DatePattern,
                L"datePattern") {}

CXFA_DatePattern::~CXFA_DatePattern() {}

WideStringView CXFA_DatePattern::GetName() const {
  return L"datePattern";
}

XFA_Element CXFA_DatePattern::GetElementType() const {
  return XFA_Element::DatePattern;
}
