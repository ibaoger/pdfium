// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_present.h"

// static
bool CXFA_Present::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Present::CXFA_Present(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Present,
                L"present") {}

CXFA_Present::~CXFA_Present() {}

WideStringView CXFA_Present::GetName() const {
  return L"present";
}

XFA_Element CXFA_Present::GetElementType() const {
  return XFA_Element::Present;
}
