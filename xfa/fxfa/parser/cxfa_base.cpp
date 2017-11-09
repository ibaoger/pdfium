// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_base.h"

// static
bool CXFA_Base::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Base::CXFA_Base(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Base,
                L"base") {}

CXFA_Base::~CXFA_Base() {}

WideStringView CXFA_Base::GetName() const {
  return L"base";
}

XFA_Element CXFA_Base::GetElementType() const {
  return XFA_Element::Base;
}
