// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_submiturl.h"

// static
bool CXFA_SubmitUrl::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_SubmitUrl::CXFA_SubmitUrl(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::SubmitUrl,
                L"submitUrl") {}

CXFA_SubmitUrl::~CXFA_SubmitUrl() {}

WideStringView CXFA_SubmitUrl::GetName() const {
  return L"submitUrl";
}

XFA_Element CXFA_SubmitUrl::GetElementType() const {
  return XFA_Element::SubmitUrl;
}
