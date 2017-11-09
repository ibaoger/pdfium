// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_execute.h"

// static
bool CXFA_Execute::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Execute::CXFA_Execute(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Execute,
                L"execute") {}

CXFA_Execute::~CXFA_Execute() {}

WideStringView CXFA_Execute::GetName() const {
  return L"execute";
}

XFA_Element CXFA_Execute::GetElementType() const {
  return XFA_Element::Execute;
}
