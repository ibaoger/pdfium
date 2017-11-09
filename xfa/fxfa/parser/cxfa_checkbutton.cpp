// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_checkbutton.h"

// static
bool CXFA_CheckButton::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_CheckButton::CXFA_CheckButton(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::CheckButton,
                L"checkButton") {}

CXFA_CheckButton::~CXFA_CheckButton() {}

WideStringView CXFA_CheckButton::GetName() const {
  return L"checkButton";
}

XFA_Element CXFA_CheckButton::GetElementType() const {
  return XFA_Element::CheckButton;
}
