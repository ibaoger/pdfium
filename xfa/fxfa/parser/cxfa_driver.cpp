// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_driver.h"

// static
bool CXFA_Driver::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Driver::CXFA_Driver(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Driver,
                L"driver") {}

CXFA_Driver::~CXFA_Driver() {}

WideStringView CXFA_Driver::GetName() const {
  return L"driver";
}

XFA_Element CXFA_Driver::GetElementType() const {
  return XFA_Element::Driver;
}
