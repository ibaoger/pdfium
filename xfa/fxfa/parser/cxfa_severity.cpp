// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_severity.h"

// static
bool CXFA_Severity::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Severity::CXFA_Severity(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::Severity,
                L"severity") {}

CXFA_Severity::~CXFA_Severity() {}

WideStringView CXFA_Severity::GetName() const {
  return L"severity";
}

XFA_Element CXFA_Severity::GetElementType() const {
  return XFA_Element::Severity;
}
