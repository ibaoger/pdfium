// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_xsl.h"

// static
bool CXFA_Xsl::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Xsl::CXFA_Xsl(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Xsl, L"xsl") {}

CXFA_Xsl::~CXFA_Xsl() {}

WideStringView CXFA_Xsl::GetName() const {
  return L"xsl";
}

XFA_Element CXFA_Xsl::GetElementType() const {
  return XFA_Element::Xsl;
}
