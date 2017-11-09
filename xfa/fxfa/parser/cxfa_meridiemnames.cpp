// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_meridiemnames.h"

// static
bool CXFA_MeridiemNames::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_MeridiemNames::CXFA_MeridiemNames(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::MeridiemNames,
                L"meridiemNames") {}

CXFA_MeridiemNames::~CXFA_MeridiemNames() {}

WideStringView CXFA_MeridiemNames::GetName() const {
  return L"meridiemNames";
}

XFA_Element CXFA_MeridiemNames::GetElementType() const {
  return XFA_Element::MeridiemNames;
}
