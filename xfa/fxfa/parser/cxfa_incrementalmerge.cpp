// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_incrementalmerge.h"

// static
bool CXFA_IncrementalMerge::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_IncrementalMerge::CXFA_IncrementalMerge(CXFA_Document* doc,
                                             XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::IncrementalMerge,
                L"incrementalMerge") {}

CXFA_IncrementalMerge::~CXFA_IncrementalMerge() {}

WideStringView CXFA_IncrementalMerge::GetName() const {
  return L"incrementalMerge";
}

XFA_Element CXFA_IncrementalMerge::GetElementType() const {
  return XFA_Element::IncrementalMerge;
}
