// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_connectionset.h"

// static
bool CXFA_ConnectionSet::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_ConnectionSet & packet);
}

CXFA_ConnectionSet::CXFA_ConnectionSet(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ModelNode,
                XFA_Element::ConnectionSet,
                L"connectionSet") {}

CXFA_ConnectionSet::~CXFA_ConnectionSet() {}

WideStringView CXFA_ConnectionSet::GetName() const {
  return L"connectionSet";
}

XFA_Element CXFA_ConnectionSet::GetElementType() const {
  return XFA_Element::ConnectionSet;
}
