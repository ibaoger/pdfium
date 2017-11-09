// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_stipple.h"

// static
bool CXFA_Stipple::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Stipple::CXFA_Stipple(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Stipple,
                L"stipple") {}

CXFA_Stipple::~CXFA_Stipple() {}

WideStringView CXFA_Stipple::GetName() const {
  return L"stipple";
}

XFA_Element CXFA_Stipple::GetElementType() const {
  return XFA_Element::Stipple;
}
