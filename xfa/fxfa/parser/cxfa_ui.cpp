// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_ui.h"

// static
bool CXFA_Ui::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Ui::CXFA_Ui(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Ui, L"ui") {}

CXFA_Ui::~CXFA_Ui() {}

WideStringView CXFA_Ui::GetName() const {
  return L"ui";
}

XFA_Element CXFA_Ui::GetElementType() const {
  return XFA_Element::Ui;
}
