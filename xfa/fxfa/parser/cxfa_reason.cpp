// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_reason.h"

// static
bool CXFA_Reason::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Reason::CXFA_Reason(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::Reason,
                L"reason") {}

CXFA_Reason::~CXFA_Reason() {}

WideStringView CXFA_Reason::GetName() const {
  return L"reason";
}

XFA_Element CXFA_Reason::GetElementType() const {
  return XFA_Element::Reason;
}
