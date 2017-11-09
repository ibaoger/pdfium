// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_list.h"

// static
bool CXFA_List::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_XDP & packet);
}

CXFA_List::CXFA_List(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::List, XFA_Element::List, L"list") {
}

CXFA_List::~CXFA_List() {}

WideStringView CXFA_List::GetName() const {
  return L"list";
}

XFA_Element CXFA_List::GetElementType() const {
  return XFA_Element::List;
}
