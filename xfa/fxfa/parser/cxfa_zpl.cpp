// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_zpl.h"

// static
bool CXFA_Zpl::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Zpl::CXFA_Zpl(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Zpl, L"zpl") {}

CXFA_Zpl::~CXFA_Zpl() {}

WideStringView CXFA_Zpl::GetName() const {
  return L"zpl";
}

XFA_Element CXFA_Zpl::GetElementType() const {
  return XFA_Element::Zpl;
}
