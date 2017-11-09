// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_password.h"

// static
bool CXFA_Password::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_SourceSet & packet);
}

CXFA_Password::CXFA_Password(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::Password,
                L"password") {}

CXFA_Password::~CXFA_Password() {}

WideStringView CXFA_Password::GetName() const {
  return L"password";
}

XFA_Element CXFA_Password::GetElementType() const {
  return XFA_Element::Password;
}
