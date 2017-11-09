// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_calculate.h"

// static
bool CXFA_Calculate::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Calculate::CXFA_Calculate(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Calculate,
                L"calculate") {}

CXFA_Calculate::~CXFA_Calculate() {}

WideStringView CXFA_Calculate::GetName() const {
  return L"calculate";
}

XFA_Element CXFA_Calculate::GetElementType() const {
  return XFA_Element::Calculate;
}
