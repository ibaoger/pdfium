// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_producer.h"

// static
bool CXFA_Producer::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Producer::CXFA_Producer(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Producer,
                L"producer") {}

CXFA_Producer::~CXFA_Producer() {}

WideStringView CXFA_Producer::GetName() const {
  return L"producer";
}

XFA_Element CXFA_Producer::GetElementType() const {
  return XFA_Element::Producer;
}
