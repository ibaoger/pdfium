// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_rename.h"

// static
bool CXFA_Rename::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Rename::CXFA_Rename(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::Rename,
                L"rename") {}

CXFA_Rename::~CXFA_Rename() {}

WideStringView CXFA_Rename::GetName() const {
  return L"rename";
}

XFA_Element CXFA_Rename::GetElementType() const {
  return XFA_Element::Rename;
}
