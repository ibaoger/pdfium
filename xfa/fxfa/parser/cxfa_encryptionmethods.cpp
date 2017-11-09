// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_encryptionmethods.h"

// static
bool CXFA_EncryptionMethods::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_EncryptionMethods::CXFA_EncryptionMethods(CXFA_Document* doc,
                                               XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::EncryptionMethods,
                L"encryptionMethods") {}

CXFA_EncryptionMethods::~CXFA_EncryptionMethods() {}

WideStringView CXFA_EncryptionMethods::GetName() const {
  return L"encryptionMethods";
}

XFA_Element CXFA_EncryptionMethods::GetElementType() const {
  return XFA_Element::EncryptionMethods;
}
