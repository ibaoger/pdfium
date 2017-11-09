// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_encodings.h"

// static
bool CXFA_Encodings::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Encodings::CXFA_Encodings(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Encodings,
                L"encodings") {}

CXFA_Encodings::~CXFA_Encodings() {}

WideStringView CXFA_Encodings::GetName() const {
  return L"encodings";
}

XFA_Element CXFA_Encodings::GetElementType() const {
  return XFA_Element::Encodings;
}
