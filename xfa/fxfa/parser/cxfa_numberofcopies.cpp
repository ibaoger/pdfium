// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_numberofcopies.h"

// static
bool CXFA_NumberOfCopies::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_NumberOfCopies::CXFA_NumberOfCopies(CXFA_Document* doc,
                                         XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::NumberOfCopies,
                L"numberOfCopies") {}

CXFA_NumberOfCopies::~CXFA_NumberOfCopies() {}

WideStringView CXFA_NumberOfCopies::GetName() const {
  return L"numberOfCopies";
}

XFA_Element CXFA_NumberOfCopies::GetElementType() const {
  return XFA_Element::NumberOfCopies;
}
