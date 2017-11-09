// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_reasons.h"

// static
bool CXFA_Reasons::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Reasons::CXFA_Reasons(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Reasons,
                L"reasons") {}

CXFA_Reasons::~CXFA_Reasons() {}

WideStringView CXFA_Reasons::GetName() const {
  return L"reasons";
}

XFA_Element CXFA_Reasons::GetElementType() const {
  return XFA_Element::Reasons;
}
