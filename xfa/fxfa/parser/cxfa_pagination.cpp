// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pagination.h"

// static
bool CXFA_Pagination::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Pagination::CXFA_Pagination(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Pagination,
                L"pagination") {}

CXFA_Pagination::~CXFA_Pagination() {}

WideStringView CXFA_Pagination::GetName() const {
  return L"pagination";
}

XFA_Element CXFA_Pagination::GetElementType() const {
  return XFA_Element::Pagination;
}
