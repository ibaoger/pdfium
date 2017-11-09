// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_signdata.h"

// static
bool CXFA_SignData::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_SignData::CXFA_SignData(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::SignData,
                L"signData") {}

CXFA_SignData::~CXFA_SignData() {}

WideStringView CXFA_SignData::GetName() const {
  return L"signData";
}

XFA_Element CXFA_SignData::GetElementType() const {
  return XFA_Element::SignData;
}
