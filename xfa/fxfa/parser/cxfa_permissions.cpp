// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_permissions.h"

// static
bool CXFA_Permissions::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Permissions::CXFA_Permissions(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Permissions,
                L"permissions") {}

CXFA_Permissions::~CXFA_Permissions() {}

WideStringView CXFA_Permissions::GetName() const {
  return L"permissions";
}

XFA_Element CXFA_Permissions::GetElementType() const {
  return XFA_Element::Permissions;
}
