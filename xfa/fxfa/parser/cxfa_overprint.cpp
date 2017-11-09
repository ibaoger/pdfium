// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_overprint.h"

// static
bool CXFA_Overprint::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Overprint::CXFA_Overprint(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Overprint,
                L"overprint") {}

CXFA_Overprint::~CXFA_Overprint() {}

WideStringView CXFA_Overprint::GetName() const {
  return L"overprint";
}

XFA_Element CXFA_Overprint::GetElementType() const {
  return XFA_Element::Overprint;
}
