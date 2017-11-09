// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_map.h"

// static
bool CXFA_Map::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Config | XFA_XDPPACKET_SourceSet) & packet);
}

CXFA_Map::CXFA_Map(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Map, L"map") {}

CXFA_Map::~CXFA_Map() {}

WideStringView CXFA_Map::GetName() const {
  return L"map";
}

XFA_Element CXFA_Map::GetElementType() const {
  return XFA_Element::Map;
}
