// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_mode.h"

// static
bool CXFA_Mode::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Mode::CXFA_Mode(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Mode,
                L"mode") {}

CXFA_Mode::~CXFA_Mode() {}

WideStringView CXFA_Mode::GetName() const {
  return L"mode";
}

XFA_Element CXFA_Mode::GetElementType() const {
  return XFA_Element::Mode;
}
