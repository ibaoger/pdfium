// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_eranames.h"

// static
bool CXFA_EraNames::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_EraNames::CXFA_EraNames(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::EraNames,
                L"eraNames") {}

CXFA_EraNames::~CXFA_EraNames() {}

WideStringView CXFA_EraNames::GetName() const {
  return L"eraNames";
}

XFA_Element CXFA_EraNames::GetElementType() const {
  return XFA_Element::EraNames;
}
