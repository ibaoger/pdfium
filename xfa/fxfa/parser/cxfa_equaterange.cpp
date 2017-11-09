// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_equaterange.h"

// static
bool CXFA_EquateRange::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_EquateRange::CXFA_EquateRange(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::EquateRange,
                L"equateRange") {}

CXFA_EquateRange::~CXFA_EquateRange() {}

WideStringView CXFA_EquateRange::GetName() const {
  return L"equateRange";
}

XFA_Element CXFA_EquateRange::GetElementType() const {
  return XFA_Element::EquateRange;
}
