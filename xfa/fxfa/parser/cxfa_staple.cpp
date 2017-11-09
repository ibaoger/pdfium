// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_staple.h"

// static
bool CXFA_Staple::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Staple::CXFA_Staple(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Staple,
                L"staple") {}

CXFA_Staple::~CXFA_Staple() {}

WideStringView CXFA_Staple::GetName() const {
  return L"staple";
}

XFA_Element CXFA_Staple::GetElementType() const {
  return XFA_Element::Staple;
}
