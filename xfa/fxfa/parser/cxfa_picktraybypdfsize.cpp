// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_picktraybypdfsize.h"

// static
bool CXFA_PickTrayByPDFSize::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_PickTrayByPDFSize::CXFA_PickTrayByPDFSize(CXFA_Document* doc,
                                               XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::PickTrayByPDFSize,
                L"pickTrayByPDFSize") {}

CXFA_PickTrayByPDFSize::~CXFA_PickTrayByPDFSize() {}

WideStringView CXFA_PickTrayByPDFSize::GetName() const {
  return L"pickTrayByPDFSize";
}

XFA_Element CXFA_PickTrayByPDFSize::GetElementType() const {
  return XFA_Element::PickTrayByPDFSize;
}
