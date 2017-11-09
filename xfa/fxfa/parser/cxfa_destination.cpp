// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_destination.h"

// static
bool CXFA_Destination::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Destination::CXFA_Destination(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Destination,
                L"destination") {}

CXFA_Destination::~CXFA_Destination() {}

WideStringView CXFA_Destination::GetName() const {
  return L"destination";
}

XFA_Element CXFA_Destination::GetElementType() const {
  return XFA_Element::Destination;
}
