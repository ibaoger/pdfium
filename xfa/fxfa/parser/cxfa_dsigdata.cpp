// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_dsigdata.h"

// static
bool CXFA_DSigData::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_DSigData::CXFA_DSigData(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::DSigData,
                L"dSigData") {}

CXFA_DSigData::~CXFA_DSigData() {}

WideStringView CXFA_DSigData::GetName() const {
  return L"dSigData";
}

XFA_Element CXFA_DSigData::GetElementType() const {
  return XFA_Element::DSigData;
}
