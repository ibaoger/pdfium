// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pagerange.h"

// static
bool CXFA_PageRange::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_PageRange::CXFA_PageRange(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::PageRange,
                L"pageRange") {}

CXFA_PageRange::~CXFA_PageRange() {}

WideStringView CXFA_PageRange::GetName() const {
  return L"pageRange";
}

XFA_Element CXFA_PageRange::GetElementType() const {
  return XFA_Element::PageRange;
}
