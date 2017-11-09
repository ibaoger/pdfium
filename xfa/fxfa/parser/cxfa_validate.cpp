// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_validate.h"

// static
bool CXFA_Validate::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(
      (XFA_XDPPACKET_Config | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) &
      packet);
}

CXFA_Validate::CXFA_Validate(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Validate,
                L"validate") {}

CXFA_Validate::~CXFA_Validate() {}

WideStringView CXFA_Validate::GetName() const {
  return L"validate";
}

XFA_Element CXFA_Validate::GetElementType() const {
  return XFA_Element::Validate;
}
