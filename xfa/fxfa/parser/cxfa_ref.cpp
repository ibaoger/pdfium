// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_ref.h"

// static
bool CXFA_Ref::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Ref::CXFA_Ref(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::Ref,
                L"ref") {}

CXFA_Ref::~CXFA_Ref() {}

WideStringView CXFA_Ref::GetName() const {
  return L"ref";
}

XFA_Element CXFA_Ref::GetElementType() const {
  return XFA_Element::Ref;
}
