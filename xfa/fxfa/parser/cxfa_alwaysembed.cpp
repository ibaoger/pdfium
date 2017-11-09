// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_alwaysembed.h"

// static
bool CXFA_AlwaysEmbed::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_AlwaysEmbed::CXFA_AlwaysEmbed(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::AlwaysEmbed,
                L"alwaysEmbed") {}

CXFA_AlwaysEmbed::~CXFA_AlwaysEmbed() {}

WideStringView CXFA_AlwaysEmbed::GetName() const {
  return L"alwaysEmbed";
}

XFA_Element CXFA_AlwaysEmbed::GetElementType() const {
  return XFA_Element::AlwaysEmbed;
}
