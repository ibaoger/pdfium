// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_psmap.h"

// static
bool CXFA_PsMap::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_PsMap::CXFA_PsMap(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::PsMap,
                L"psMap") {}

CXFA_PsMap::~CXFA_PsMap() {}

WideStringView CXFA_PsMap::GetName() const {
  return L"psMap";
}

XFA_Element CXFA_PsMap::GetElementType() const {
  return XFA_Element::PsMap;
}
