// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_exclude.h"

// static
bool CXFA_Exclude::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Exclude::CXFA_Exclude(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Exclude,
                L"exclude") {}

CXFA_Exclude::~CXFA_Exclude() {}

WideStringView CXFA_Exclude::GetName() const {
  return L"exclude";
}

XFA_Element CXFA_Exclude::GetElementType() const {
  return XFA_Element::Exclude;
}
