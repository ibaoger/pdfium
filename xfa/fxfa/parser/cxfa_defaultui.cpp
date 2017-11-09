// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_defaultui.h"

// static
bool CXFA_DefaultUi::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_DefaultUi::CXFA_DefaultUi(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::DefaultUi,
                L"defaultUi") {}

CXFA_DefaultUi::~CXFA_DefaultUi() {}

WideStringView CXFA_DefaultUi::GetName() const {
  return L"defaultUi";
}

XFA_Element CXFA_DefaultUi::GetElementType() const {
  return XFA_Element::DefaultUi;
}
