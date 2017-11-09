// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_paginationoverride.h"

// static
bool CXFA_PaginationOverride::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_PaginationOverride::CXFA_PaginationOverride(CXFA_Document* doc,
                                                 XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::PaginationOverride,
                L"paginationOverride") {}

CXFA_PaginationOverride::~CXFA_PaginationOverride() {}

WideStringView CXFA_PaginationOverride::GetName() const {
  return L"paginationOverride";
}

XFA_Element CXFA_PaginationOverride::GetElementType() const {
  return XFA_Element::PaginationOverride;
}
