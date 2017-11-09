// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_eventpseudomodel.h"

// static
bool CXFA_EventPseudoModel::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_XDP & packet);
}

CXFA_EventPseudoModel::CXFA_EventPseudoModel(CXFA_Document* doc,
                                             XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Object,
                XFA_Element::EventPseudoModel,
                L"eventPseudoModel") {}

CXFA_EventPseudoModel::~CXFA_EventPseudoModel() {}

WideStringView CXFA_EventPseudoModel::GetName() const {
  return L"eventPseudoModel";
}

XFA_Element CXFA_EventPseudoModel::GetElementType() const {
  return XFA_Element::EventPseudoModel;
}
