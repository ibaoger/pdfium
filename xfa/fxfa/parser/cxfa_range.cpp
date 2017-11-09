// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_range.h"

// static
bool CXFA_Range::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Range::CXFA_Range(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::Range,
                L"range") {}

CXFA_Range::~CXFA_Range() {}

WideStringView CXFA_Range::GetName() const {
  return L"range";
}

XFA_Element CXFA_Range::GetElementType() const {
  return XFA_Element::Range;
}
