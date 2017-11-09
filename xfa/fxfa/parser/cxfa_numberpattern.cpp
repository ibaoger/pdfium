// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_numberpattern.h"

// static
bool CXFA_NumberPattern::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_NumberPattern::CXFA_NumberPattern(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::NumberPattern,
                L"numberPattern") {}

CXFA_NumberPattern::~CXFA_NumberPattern() {}

WideStringView CXFA_NumberPattern::GetName() const {
  return L"numberPattern";
}

XFA_Element CXFA_NumberPattern::GetElementType() const {
  return XFA_Element::NumberPattern;
}
