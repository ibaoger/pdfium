// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_compress.h"

// static
bool CXFA_Compress::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Compress::CXFA_Compress(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Compress,
                L"compress") {}

CXFA_Compress::~CXFA_Compress() {}

WideStringView CXFA_Compress::GetName() const {
  return L"compress";
}

XFA_Element CXFA_Compress::GetElementType() const {
  return XFA_Element::Compress;
}
