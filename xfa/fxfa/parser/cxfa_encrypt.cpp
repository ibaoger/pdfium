// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_encrypt.h"

// static
bool CXFA_Encrypt::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(
      (XFA_XDPPACKET_Template | XFA_XDPPACKET_Config | XFA_XDPPACKET_Form) &
      packet);
}

CXFA_Encrypt::CXFA_Encrypt(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Encrypt,
                L"encrypt") {}

CXFA_Encrypt::~CXFA_Encrypt() {}

WideStringView CXFA_Encrypt::GetName() const {
  return L"encrypt";
}

XFA_Element CXFA_Encrypt::GetElementType() const {
  return XFA_Element::Encrypt;
}
