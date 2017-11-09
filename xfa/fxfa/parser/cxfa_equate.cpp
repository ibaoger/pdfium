// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_equate.h"

// static
bool CXFA_Equate::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Equate::CXFA_Equate(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::Equate,
                L"equate") {}

CXFA_Equate::~CXFA_Equate() {}

WideStringView CXFA_Equate::GetName() const {
  return L"equate";
}

XFA_Element CXFA_Equate::GetElementType() const {
  return XFA_Element::Equate;
}
