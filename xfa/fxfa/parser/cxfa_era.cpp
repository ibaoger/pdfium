// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_era.h"

// static
bool CXFA_Era::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_Era::CXFA_Era(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Era,
                L"era") {}

CXFA_Era::~CXFA_Era() {}

WideStringView CXFA_Era::GetName() const {
  return L"era";
}

XFA_Element CXFA_Era::GetElementType() const {
  return XFA_Element::Era;
}
