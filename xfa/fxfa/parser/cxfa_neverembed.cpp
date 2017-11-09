// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_neverembed.h"

// static
bool CXFA_NeverEmbed::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_NeverEmbed::CXFA_NeverEmbed(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::NeverEmbed,
                L"neverEmbed") {}

CXFA_NeverEmbed::~CXFA_NeverEmbed() {}

WideStringView CXFA_NeverEmbed::GetName() const {
  return L"neverEmbed";
}

XFA_Element CXFA_NeverEmbed::GetElementType() const {
  return XFA_Element::NeverEmbed;
}
