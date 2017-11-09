// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_choicelist.h"

// static
bool CXFA_ChoiceList::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_ChoiceList::CXFA_ChoiceList(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::ChoiceList,
                L"choiceList") {}

CXFA_ChoiceList::~CXFA_ChoiceList() {}

WideStringView CXFA_ChoiceList::GetName() const {
  return L"choiceList";
}

XFA_Element CXFA_ChoiceList::GetElementType() const {
  return XFA_Element::ChoiceList;
}
