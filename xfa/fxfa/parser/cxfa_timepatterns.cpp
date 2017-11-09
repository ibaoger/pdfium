// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_timepatterns.h"

// static
bool CXFA_TimePatterns::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_TimePatterns::CXFA_TimePatterns(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::TimePatterns,
                L"timePatterns") {}

CXFA_TimePatterns::~CXFA_TimePatterns() {}

WideStringView CXFA_TimePatterns::GetName() const {
  return L"timePatterns";
}

XFA_Element CXFA_TimePatterns::GetElementType() const {
  return XFA_Element::TimePatterns;
}
