// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_threshold.h"

// static
bool CXFA_Threshold::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Threshold::CXFA_Threshold(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::Threshold,
                L"threshold") {}

CXFA_Threshold::~CXFA_Threshold() {}

WideStringView CXFA_Threshold::GetName() const {
  return L"threshold";
}

XFA_Element CXFA_Threshold::GetElementType() const {
  return XFA_Element::Threshold;
}
