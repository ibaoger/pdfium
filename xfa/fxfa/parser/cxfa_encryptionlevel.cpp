// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_encryptionlevel.h"

// static
bool CXFA_EncryptionLevel::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_EncryptionLevel::CXFA_EncryptionLevel(CXFA_Document* doc,
                                           XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::EncryptionLevel,
                L"encryptionLevel") {}

CXFA_EncryptionLevel::~CXFA_EncryptionLevel() {}

WideStringView CXFA_EncryptionLevel::GetName() const {
  return L"encryptionLevel";
}

XFA_Element CXFA_EncryptionLevel::GetElementType() const {
  return XFA_Element::EncryptionLevel;
}
