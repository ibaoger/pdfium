// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_nameattr.h"

// static
bool CXFA_NameAttr::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_NameAttr::CXFA_NameAttr(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::NameAttr,
                L"nameAttr") {}

CXFA_NameAttr::~CXFA_NameAttr() {}

WideStringView CXFA_NameAttr::GetName() const {
  return L"nameAttr";
}

XFA_Element CXFA_NameAttr::GetElementType() const {
  return XFA_Element::NameAttr;
}
