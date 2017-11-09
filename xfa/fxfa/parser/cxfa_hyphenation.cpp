// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_hyphenation.h"

// static
bool CXFA_Hyphenation::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Hyphenation::CXFA_Hyphenation(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Hyphenation,
                L"hyphenation") {}

CXFA_Hyphenation::~CXFA_Hyphenation() {}

WideStringView CXFA_Hyphenation::GetName() const {
  return L"hyphenation";
}

XFA_Element CXFA_Hyphenation::GetElementType() const {
  return XFA_Element::Hyphenation;
}
