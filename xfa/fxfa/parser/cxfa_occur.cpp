// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_occur.h"

// static
bool CXFA_Occur::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Occur::CXFA_Occur(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Occur,
                L"occur") {}

CXFA_Occur::~CXFA_Occur() {}

WideStringView CXFA_Occur::GetName() const {
  return L"occur";
}

XFA_Element CXFA_Occur::GetElementType() const {
  return XFA_Element::Occur;
}
