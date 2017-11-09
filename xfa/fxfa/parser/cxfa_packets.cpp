// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_packets.h"

// static
bool CXFA_Packets::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Packets::CXFA_Packets(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Packets,
                L"packets") {}

CXFA_Packets::~CXFA_Packets() {}

WideStringView CXFA_Packets::GetName() const {
  return L"packets";
}

XFA_Element CXFA_Packets::GetElementType() const {
  return XFA_Element::Packets;
}
