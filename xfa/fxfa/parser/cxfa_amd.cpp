// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_amd.h"

// static
bool CXFA_Amd::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Amd::CXFA_Amd(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Amd,
                L"amd") {}

CXFA_Amd::~CXFA_Amd() {}

WideStringView CXFA_Amd::GetName() const {
  return L"amd";
}

XFA_Element CXFA_Amd::GetElementType() const {
  return XFA_Element::Amd;
}
