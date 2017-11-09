// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pattern.h"

// static
bool CXFA_Pattern::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Pattern::CXFA_Pattern(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Pattern,
                L"pattern") {}

CXFA_Pattern::~CXFA_Pattern() {}

WideStringView CXFA_Pattern::GetName() const {
  return L"pattern";
}

XFA_Element CXFA_Pattern::GetElementType() const {
  return XFA_Element::Pattern;
}
