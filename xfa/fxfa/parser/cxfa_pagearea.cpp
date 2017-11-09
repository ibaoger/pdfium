// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pagearea.h"

// static
bool CXFA_PageArea::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_PageArea::CXFA_PageArea(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContainerNode,
                XFA_Element::PageArea,
                L"pageArea") {}

CXFA_PageArea::~CXFA_PageArea() {}

WideStringView CXFA_PageArea::GetName() const {
  return L"pageArea";
}

XFA_Element CXFA_PageArea::GetElementType() const {
  return XFA_Element::PageArea;
}
