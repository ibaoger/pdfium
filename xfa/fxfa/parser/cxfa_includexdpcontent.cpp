// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_includexdpcontent.h"

// static
bool CXFA_IncludeXDPContent::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_IncludeXDPContent::CXFA_IncludeXDPContent(CXFA_Document* doc,
                                               XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::IncludeXDPContent,
                L"includeXDPContent") {}

CXFA_IncludeXDPContent::~CXFA_IncludeXDPContent() {}

WideStringView CXFA_IncludeXDPContent::GetName() const {
  return L"includeXDPContent";
}

XFA_Element CXFA_IncludeXDPContent::GetElementType() const {
  return XFA_Element::IncludeXDPContent;
}
