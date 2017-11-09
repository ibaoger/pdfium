// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_documentassembly.h"

// static
bool CXFA_DocumentAssembly::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_DocumentAssembly::CXFA_DocumentAssembly(CXFA_Document* doc,
                                             XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::DocumentAssembly,
                L"documentAssembly") {}

CXFA_DocumentAssembly::~CXFA_DocumentAssembly() {}

WideStringView CXFA_DocumentAssembly::GetName() const {
  return L"documentAssembly";
}

XFA_Element CXFA_DocumentAssembly::GetElementType() const {
  return XFA_Element::DocumentAssembly;
}
