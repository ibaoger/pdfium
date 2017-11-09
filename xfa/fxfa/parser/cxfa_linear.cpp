// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_linear.h"

// static
bool CXFA_Linear::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Linear::CXFA_Linear(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Linear,
                L"linear") {}

CXFA_Linear::~CXFA_Linear() {}

WideStringView CXFA_Linear::GetName() const {
  return L"linear";
}

XFA_Element CXFA_Linear::GetElementType() const {
  return XFA_Element::Linear;
}
