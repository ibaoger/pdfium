// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_msgid.h"

// static
bool CXFA_MsgId::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_MsgId::CXFA_MsgId(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::MsgId,
                L"msgId") {}

CXFA_MsgId::~CXFA_MsgId() {}

WideStringView CXFA_MsgId::GetName() const {
  return L"msgId";
}

XFA_Element CXFA_MsgId::GetElementType() const {
  return XFA_Element::MsgId;
}
