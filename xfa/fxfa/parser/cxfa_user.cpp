// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_user.h"

// static
bool CXFA_User::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_SourceSet & packet);
}

CXFA_User::CXFA_User(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::User,
                L"user") {}

CXFA_User::~CXFA_User() {}

WideStringView CXFA_User::GetName() const {
  return L"user";
}

XFA_Element CXFA_User::GetElementType() const {
  return XFA_Element::User;
}
