// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_insert.h"

// static
bool CXFA_Insert::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_SourceSet & packet);
}

CXFA_Insert::CXFA_Insert(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::Insert,
                L"insert") {}

CXFA_Insert::~CXFA_Insert() {}

WideStringView CXFA_Insert::GetName() const {
  return L"insert";
}

XFA_Element CXFA_Insert::GetElementType() const {
  return XFA_Element::Insert;
}
