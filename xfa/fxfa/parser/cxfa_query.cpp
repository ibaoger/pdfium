// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_query.h"

// static
bool CXFA_Query::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_SourceSet & packet);
}

CXFA_Query::CXFA_Query(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Query,
                L"query") {}

CXFA_Query::~CXFA_Query() {}

WideStringView CXFA_Query::GetName() const {
  return L"query";
}

XFA_Element CXFA_Query::GetElementType() const {
  return XFA_Element::Query;
}
