// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_signature.h"

// static
bool CXFA_Signature::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Signature::CXFA_Signature(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Signature,
                L"signature") {}

CXFA_Signature::~CXFA_Signature() {}

WideStringView CXFA_Signature::GetName() const {
  return L"signature";
}

XFA_Element CXFA_Signature::GetElementType() const {
  return XFA_Element::Signature;
}
