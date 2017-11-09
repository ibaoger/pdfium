// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_layoutpseudomodel.h"

// static
bool CXFA_LayoutPseudoModel::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_XDP & packet);
}

CXFA_LayoutPseudoModel::CXFA_LayoutPseudoModel(CXFA_Document* doc,
                                               XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Object,
                XFA_Element::LayoutPseudoModel,
                L"layoutPseudoModel") {}

CXFA_LayoutPseudoModel::~CXFA_LayoutPseudoModel() {}

WideStringView CXFA_LayoutPseudoModel::GetName() const {
  return L"layoutPseudoModel";
}

XFA_Element CXFA_LayoutPseudoModel::GetElementType() const {
  return XFA_Element::LayoutPseudoModel;
}
