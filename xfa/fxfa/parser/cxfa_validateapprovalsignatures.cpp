// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_validateapprovalsignatures.h"

// static
bool CXFA_ValidateApprovalSignatures::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_ValidateApprovalSignatures::CXFA_ValidateApprovalSignatures(
    CXFA_Document* doc,
    XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::ValidateApprovalSignatures,
                L"validateApprovalSignatures") {}

CXFA_ValidateApprovalSignatures::~CXFA_ValidateApprovalSignatures() {}

WideStringView CXFA_ValidateApprovalSignatures::GetName() const {
  return L"validateApprovalSignatures";
}

XFA_Element CXFA_ValidateApprovalSignatures::GetElementType() const {
  return XFA_Element::ValidateApprovalSignatures;
}
