// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_area.h"

// static
bool CXFA_Area::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(
      (XFA_XDPPACKET_Config | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) &
      packet);
}

CXFA_Area::CXFA_Area(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContainerNode,
                XFA_Element::Area,
                L"area") {}

CXFA_Area::~CXFA_Area() {}

WideStringView CXFA_Area::GetName() const {
  return L"area";
}

XFA_Element CXFA_Area::GetElementType() const {
  return XFA_Element::Area;
}
