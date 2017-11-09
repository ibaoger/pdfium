// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_logpseudomodel.h"

// static
bool CXFA_LogPseudoModel::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_XDP & packet);
}

CXFA_LogPseudoModel::CXFA_LogPseudoModel(CXFA_Document* doc,
                                         XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Object,
                XFA_Element::LogPseudoModel,
                L"logPseudoModel") {}

CXFA_LogPseudoModel::~CXFA_LogPseudoModel() {}

WideStringView CXFA_LogPseudoModel::GetName() const {
  return L"logPseudoModel";
}

XFA_Element CXFA_LogPseudoModel::GetElementType() const {
  return XFA_Element::LogPseudoModel;
}
