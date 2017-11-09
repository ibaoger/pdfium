// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_hostpseudomodel.h"

// static
bool CXFA_HostPseudoModel::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_XDP & packet);
}

CXFA_HostPseudoModel::CXFA_HostPseudoModel(CXFA_Document* doc,
                                           XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Object,
                XFA_Element::HostPseudoModel,
                L"hostPseudoModel") {}

CXFA_HostPseudoModel::~CXFA_HostPseudoModel() {}

WideStringView CXFA_HostPseudoModel::GetName() const {
  return L"hostPseudoModel";
}

XFA_Element CXFA_HostPseudoModel::GetElementType() const {
  return XFA_Element::HostPseudoModel;
}
