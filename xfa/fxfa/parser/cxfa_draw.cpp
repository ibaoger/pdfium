// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_draw.h"

// static
bool CXFA_Draw::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Draw::CXFA_Draw(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContainerNode,
                XFA_Element::Draw,
                L"draw") {}

CXFA_Draw::~CXFA_Draw() {}

WideStringView CXFA_Draw::GetName() const {
  return L"draw";
}

XFA_Element CXFA_Draw::GetElementType() const {
  return XFA_Element::Draw;
}
