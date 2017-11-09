// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_digestmethods.h"

// static
bool CXFA_DigestMethods::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_DigestMethods::CXFA_DigestMethods(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::DigestMethods,
                L"digestMethods") {}

CXFA_DigestMethods::~CXFA_DigestMethods() {}

WideStringView CXFA_DigestMethods::GetName() const {
  return L"digestMethods";
}

XFA_Element CXFA_DigestMethods::GetElementType() const {
  return XFA_Element::DigestMethods;
}
