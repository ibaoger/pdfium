// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_medium.h"

// static
bool CXFA_Medium::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Medium::CXFA_Medium(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Medium,
                L"medium") {}

CXFA_Medium::~CXFA_Medium() {}

WideStringView CXFA_Medium::GetName() const {
  return L"medium";
}

XFA_Element CXFA_Medium::GetElementType() const {
  return XFA_Element::Medium;
}
