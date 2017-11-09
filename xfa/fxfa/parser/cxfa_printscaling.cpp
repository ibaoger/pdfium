// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_printscaling.h"

// static
bool CXFA_PrintScaling::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_PrintScaling::CXFA_PrintScaling(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::PrintScaling,
                L"printScaling") {}

CXFA_PrintScaling::~CXFA_PrintScaling() {}

WideStringView CXFA_PrintScaling::GetName() const {
  return L"printScaling";
}

XFA_Element CXFA_PrintScaling::GetElementType() const {
  return XFA_Element::PrintScaling;
}
