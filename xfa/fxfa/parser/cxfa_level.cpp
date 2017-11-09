// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_level.h"

// static
bool CXFA_Level::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Level::CXFA_Level(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Level,
                L"level") {}

CXFA_Level::~CXFA_Level() {}

WideStringView CXFA_Level::GetName() const {
  return L"level";
}

XFA_Element CXFA_Level::GetElementType() const {
  return XFA_Element::Level;
}
