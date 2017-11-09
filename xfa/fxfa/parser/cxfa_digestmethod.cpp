// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_digestmethod.h"

// static
bool CXFA_DigestMethod::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_DigestMethod::CXFA_DigestMethod(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeC,
                XFA_Element::DigestMethod,
                L"digestMethod") {}

CXFA_DigestMethod::~CXFA_DigestMethod() {}

WideStringView CXFA_DigestMethod::GetName() const {
  return L"digestMethod";
}

XFA_Element CXFA_DigestMethod::GetElementType() const {
  return XFA_Element::DigestMethod;
}
