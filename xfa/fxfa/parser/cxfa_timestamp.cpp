// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_timestamp.h"

// static
bool CXFA_TimeStamp::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_TimeStamp::CXFA_TimeStamp(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::TimeStamp,
                L"timeStamp") {}

CXFA_TimeStamp::~CXFA_TimeStamp() {}

WideStringView CXFA_TimeStamp::GetName() const {
  return L"timeStamp";
}

XFA_Element CXFA_TimeStamp::GetElementType() const {
  return XFA_Element::TimeStamp;
}
