// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_exclgroup.h"

// static
bool CXFA_ExclGroup::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_ExclGroup::CXFA_ExclGroup(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContainerNode,
                XFA_Element::ExclGroup,
                L"exclGroup") {}

CXFA_ExclGroup::~CXFA_ExclGroup() {}

WideStringView CXFA_ExclGroup::GetName() const {
  return L"exclGroup";
}

XFA_Element CXFA_ExclGroup::GetElementType() const {
  return XFA_Element::ExclGroup;
}
