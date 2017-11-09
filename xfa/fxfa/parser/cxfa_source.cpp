// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_source.h"

// static
bool CXFA_Source::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_SourceSet & packet);
}

CXFA_Source::CXFA_Source(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Source,
                L"source") {}

CXFA_Source::~CXFA_Source() {}

WideStringView CXFA_Source::GetName() const {
  return L"source";
}

XFA_Element CXFA_Source::GetElementType() const {
  return XFA_Element::Source;
}
