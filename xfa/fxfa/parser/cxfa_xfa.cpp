// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_xfa.h"

// static
bool CXFA_Xfa::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_XDP & packet);
}

CXFA_Xfa::CXFA_Xfa(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ModelNode,
                XFA_Element::Xfa,
                L"xfa") {}

CXFA_Xfa::~CXFA_Xfa() {}

WideStringView CXFA_Xfa::GetName() const {
  return L"xfa";
}

XFA_Element CXFA_Xfa::GetElementType() const {
  return XFA_Element::Xfa;
}
