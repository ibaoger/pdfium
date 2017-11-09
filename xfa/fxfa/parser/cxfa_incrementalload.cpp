// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_incrementalload.h"

// static
bool CXFA_IncrementalLoad::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_IncrementalLoad::CXFA_IncrementalLoad(CXFA_Document* doc,
                                           XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::IncrementalLoad,
                L"incrementalLoad") {}

CXFA_IncrementalLoad::~CXFA_IncrementalLoad() {}

WideStringView CXFA_IncrementalLoad::GetName() const {
  return L"incrementalLoad";
}

XFA_Element CXFA_IncrementalLoad::GetElementType() const {
  return XFA_Element::IncrementalLoad;
}
