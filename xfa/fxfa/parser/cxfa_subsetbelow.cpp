// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_subsetbelow.h"

// static
bool CXFA_SubsetBelow::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_SubsetBelow::CXFA_SubsetBelow(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::SubsetBelow,
                L"subsetBelow") {}

CXFA_SubsetBelow::~CXFA_SubsetBelow() {}

WideStringView CXFA_SubsetBelow::GetName() const {
  return L"subsetBelow";
}

XFA_Element CXFA_SubsetBelow::GetElementType() const {
  return XFA_Element::SubsetBelow;
}
