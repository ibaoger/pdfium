// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_layout.h"

// static
bool CXFA_Layout::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Layout::CXFA_Layout(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Layout,
                L"layout") {}

CXFA_Layout::~CXFA_Layout() {}

WideStringView CXFA_Layout::GetName() const {
  return L"layout";
}

XFA_Element CXFA_Layout::GetElementType() const {
  return XFA_Element::Layout;
}
