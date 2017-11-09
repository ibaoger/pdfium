// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pageoffset.h"

// static
bool CXFA_PageOffset::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_PageOffset::CXFA_PageOffset(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::PageOffset,
                L"pageOffset") {}

CXFA_PageOffset::~CXFA_PageOffset() {}

WideStringView CXFA_PageOffset::GetName() const {
  return L"pageOffset";
}

XFA_Element CXFA_PageOffset::GetElementType() const {
  return XFA_Element::PageOffset;
}
