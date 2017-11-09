// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_adjustdata.h"

// static
bool CXFA_AdjustData::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_AdjustData::CXFA_AdjustData(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::AdjustData,
                L"adjustData") {}

CXFA_AdjustData::~CXFA_AdjustData() {}

WideStringView CXFA_AdjustData::GetName() const {
  return L"adjustData";
}

XFA_Element CXFA_AdjustData::GetElementType() const {
  return XFA_Element::AdjustData;
}
