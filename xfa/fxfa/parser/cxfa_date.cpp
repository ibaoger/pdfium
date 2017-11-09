// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_date.h"

// static
bool CXFA_Date::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Date::CXFA_Date(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Date,
                L"date") {}

CXFA_Date::~CXFA_Date() {}

WideStringView CXFA_Date::GetName() const {
  return L"date";
}

XFA_Element CXFA_Date::GetElementType() const {
  return XFA_Element::Date;
}
