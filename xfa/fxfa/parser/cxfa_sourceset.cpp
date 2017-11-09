// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_sourceset.h"

// static
bool CXFA_SourceSet::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_SourceSet & packet);
}

CXFA_SourceSet::CXFA_SourceSet(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ModelNode,
                XFA_Element::SourceSet,
                L"sourceSet") {}

CXFA_SourceSet::~CXFA_SourceSet() {}

WideStringView CXFA_SourceSet::GetName() const {
  return L"sourceSet";
}

XFA_Element CXFA_SourceSet::GetElementType() const {
  return XFA_Element::SourceSet;
}
