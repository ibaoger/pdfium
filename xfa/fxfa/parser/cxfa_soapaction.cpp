// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_soapaction.h"

// static
bool CXFA_SoapAction::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_ConnectionSet & packet);
}

CXFA_SoapAction::CXFA_SoapAction(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::SoapAction,
                L"soapAction") {}

CXFA_SoapAction::~CXFA_SoapAction() {}

WideStringView CXFA_SoapAction::GetName() const {
  return L"soapAction";
}

XFA_Element CXFA_SoapAction::GetElementType() const {
  return XFA_Element::SoapAction;
}
