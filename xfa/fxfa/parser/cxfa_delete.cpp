// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_delete.h"

// static
bool CXFA_Delete::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_SourceSet & packet);
}

CXFA_Delete::CXFA_Delete(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::Delete,
                L"delete") {}

CXFA_Delete::~CXFA_Delete() {}

WideStringView CXFA_Delete::GetName() const {
  return L"delete";
}

XFA_Element CXFA_Delete::GetElementType() const {
  return XFA_Element::Delete;
}
