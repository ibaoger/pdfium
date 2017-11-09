// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_compresslogicalstructure.h"

// static
bool CXFA_CompressLogicalStructure::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_CompressLogicalStructure::CXFA_CompressLogicalStructure(
    CXFA_Document* doc,
    XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::CompressLogicalStructure,
                L"compressLogicalStructure") {}

CXFA_CompressLogicalStructure::~CXFA_CompressLogicalStructure() {}

WideStringView CXFA_CompressLogicalStructure::GetName() const {
  return L"compressLogicalStructure";
}

XFA_Element CXFA_CompressLogicalStructure::GetElementType() const {
  return XFA_Element::CompressLogicalStructure;
}
