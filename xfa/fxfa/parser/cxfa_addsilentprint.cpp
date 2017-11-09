// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_addsilentprint.h"

// static
bool CXFA_AddSilentPrint::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_AddSilentPrint::CXFA_AddSilentPrint(CXFA_Document* doc,
                                         XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::AddSilentPrint,
                L"addSilentPrint") {}

CXFA_AddSilentPrint::~CXFA_AddSilentPrint() {}

WideStringView CXFA_AddSilentPrint::GetName() const {
  return L"addSilentPrint";
}

XFA_Element CXFA_AddSilentPrint::GetElementType() const {
  return XFA_Element::AddSilentPrint;
}
