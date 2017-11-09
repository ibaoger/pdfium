// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_excludens.h"

// static
bool CXFA_ExcludeNS::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_ExcludeNS::CXFA_ExcludeNS(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::ExcludeNS,
                L"excludeNS") {}

CXFA_ExcludeNS::~CXFA_ExcludeNS() {}

WideStringView CXFA_ExcludeNS::GetName() const {
  return L"excludeNS";
}

XFA_Element CXFA_ExcludeNS::GetElementType() const {
  return XFA_Element::ExcludeNS;
}
