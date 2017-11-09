// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_validationmessaging.h"

// static
bool CXFA_ValidationMessaging::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_ValidationMessaging::CXFA_ValidationMessaging(CXFA_Document* doc,
                                                   XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::ValidationMessaging,
                L"validationMessaging") {}

CXFA_ValidationMessaging::~CXFA_ValidationMessaging() {}

WideStringView CXFA_ValidationMessaging::GetName() const {
  return L"validationMessaging";
}

XFA_Element CXFA_ValidationMessaging::GetElementType() const {
  return XFA_Element::ValidationMessaging;
}
