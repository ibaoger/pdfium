// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_bind.h"

// static
bool CXFA_Bind::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(
      (XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) &
      packet);
}

CXFA_Bind::CXFA_Bind(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Bind, L"bind") {
}

CXFA_Bind::~CXFA_Bind() {}

WideStringView CXFA_Bind::GetName() const {
  return L"bind";
}

XFA_Element CXFA_Bind::GetElementType() const {
  return XFA_Element::Bind;
}
