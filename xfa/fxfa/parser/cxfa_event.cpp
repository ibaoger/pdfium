// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_event.h"

// static
bool CXFA_Event::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Event::CXFA_Event(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Event,
                L"event") {}

CXFA_Event::~CXFA_Event() {}

WideStringView CXFA_Event::GetName() const {
  return L"event";
}

XFA_Element CXFA_Event::GetElementType() const {
  return XFA_Element::Event;
}
