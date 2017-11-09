// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_change.h"

// static
bool CXFA_Change::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Change::CXFA_Change(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Change,
                L"change") {}

CXFA_Change::~CXFA_Change() {}

WideStringView CXFA_Change::GetName() const {
  return L"change";
}

XFA_Element CXFA_Change::GetElementType() const {
  return XFA_Element::Change;
}
