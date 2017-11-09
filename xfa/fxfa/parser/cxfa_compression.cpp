// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_compression.h"

// static
bool CXFA_Compression::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Compression::CXFA_Compression(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Compression,
                L"compression") {}

CXFA_Compression::~CXFA_Compression() {}

WideStringView CXFA_Compression::GetName() const {
  return L"compression";
}

XFA_Element CXFA_Compression::GetElementType() const {
  return XFA_Element::Compression;
}
