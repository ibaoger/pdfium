// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_datavalue.h"

// static
bool CXFA_DataValue::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Datasets & packet);
}

CXFA_DataValue::CXFA_DataValue(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::DataValue,
                L"dataValue") {}

CXFA_DataValue::~CXFA_DataValue() {}

WideStringView CXFA_DataValue::GetName() const {
  return L"dataValue";
}

XFA_Element CXFA_DataValue::GetElementType() const {
  return XFA_Element::DataValue;
}
