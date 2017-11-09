// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_certificate.h"

// static
bool CXFA_Certificate::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Certificate::CXFA_Certificate(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::Certificate,
                L"certificate") {}

CXFA_Certificate::~CXFA_Certificate() {}

WideStringView CXFA_Certificate::GetName() const {
  return L"certificate";
}

XFA_Element CXFA_Certificate::GetElementType() const {
  return XFA_Element::Certificate;
}
