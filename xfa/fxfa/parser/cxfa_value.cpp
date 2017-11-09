// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_value.h"

// static
bool CXFA_Value::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Value::CXFA_Value(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Value,
                L"value") {}

CXFA_Value::~CXFA_Value() {}

WideStringView CXFA_Value::GetName() const {
  return L"value";
}

XFA_Element CXFA_Value::GetElementType() const {
  return XFA_Element::Value;
}
