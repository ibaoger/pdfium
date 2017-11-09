// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_deltas.h"

// static
bool CXFA_Deltas::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Form & packet);
}

CXFA_Deltas::CXFA_Deltas(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Object,
                XFA_Element::Deltas,
                L"deltas") {}

CXFA_Deltas::~CXFA_Deltas() {}

WideStringView CXFA_Deltas::GetName() const {
  return L"deltas";
}

XFA_Element CXFA_Deltas::GetElementType() const {
  return XFA_Element::Deltas;
}
