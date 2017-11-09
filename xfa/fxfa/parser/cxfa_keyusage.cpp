// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_keyusage.h"

// static
bool CXFA_KeyUsage::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_KeyUsage::CXFA_KeyUsage(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::KeyUsage,
                L"keyUsage") {}

CXFA_KeyUsage::~CXFA_KeyUsage() {}

WideStringView CXFA_KeyUsage::GetName() const {
  return L"keyUsage";
}

XFA_Element CXFA_KeyUsage::GetElementType() const {
  return XFA_Element::KeyUsage;
}
