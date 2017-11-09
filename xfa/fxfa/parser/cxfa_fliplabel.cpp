// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_fliplabel.h"

// static
bool CXFA_FlipLabel::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_FlipLabel::CXFA_FlipLabel(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::FlipLabel,
                L"flipLabel") {}

CXFA_FlipLabel::~CXFA_FlipLabel() {}

WideStringView CXFA_FlipLabel::GetName() const {
  return L"flipLabel";
}

XFA_Element CXFA_FlipLabel::GetElementType() const {
  return XFA_Element::FlipLabel;
}
