// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_delta.h"

// static
bool CXFA_Delta::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Form & packet);
}

CXFA_Delta::CXFA_Delta(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Object,
                XFA_Element::Delta,
                L"delta") {}

CXFA_Delta::~CXFA_Delta() {}

WideStringView CXFA_Delta::GetName() const {
  return L"delta";
}

XFA_Element CXFA_Delta::GetElementType() const {
  return XFA_Element::Delta;
}
