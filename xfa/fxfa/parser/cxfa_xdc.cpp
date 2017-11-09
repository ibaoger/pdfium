// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_xdc.h"

// static
bool CXFA_Xdc::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Config | XFA_XDPPACKET_Xdc) & packet);
}

CXFA_Xdc::CXFA_Xdc(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ModelNode,
                XFA_Element::Xdc,
                L"xdc") {}

CXFA_Xdc::~CXFA_Xdc() {}

WideStringView CXFA_Xdc::GetName() const {
  return L"xdc";
}

XFA_Element CXFA_Xdc::GetElementType() const {
  return XFA_Element::Xdc;
}
