// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_signaturepseudomodel.h"

// static
bool CXFA_SignaturePseudoModel::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_XDP & packet);
}

CXFA_SignaturePseudoModel::CXFA_SignaturePseudoModel(CXFA_Document* doc,
                                                     XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Object,
                XFA_Element::SignaturePseudoModel,
                L"signaturePseudoModel") {}

CXFA_SignaturePseudoModel::~CXFA_SignaturePseudoModel() {}

WideStringView CXFA_SignaturePseudoModel::GetName() const {
  return L"signaturePseudoModel";
}

XFA_Element CXFA_SignaturePseudoModel::GetElementType() const {
  return XFA_Element::SignaturePseudoModel;
}
