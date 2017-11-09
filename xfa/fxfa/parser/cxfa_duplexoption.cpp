// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_duplexoption.h"

// static
bool CXFA_DuplexOption::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_DuplexOption::CXFA_DuplexOption(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::DuplexOption,
                L"duplexOption") {}

CXFA_DuplexOption::~CXFA_DuplexOption() {}

WideStringView CXFA_DuplexOption::GetName() const {
  return L"duplexOption";
}

XFA_Element CXFA_DuplexOption::GetElementType() const {
  return XFA_Element::DuplexOption;
}
