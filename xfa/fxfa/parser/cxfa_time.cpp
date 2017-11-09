// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_time.h"

// static
bool CXFA_Time::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Time::CXFA_Time(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Time,
                L"time") {}

CXFA_Time::~CXFA_Time() {}

WideStringView CXFA_Time::GetName() const {
  return L"time";
}

XFA_Element CXFA_Time::GetElementType() const {
  return XFA_Element::Time;
}
