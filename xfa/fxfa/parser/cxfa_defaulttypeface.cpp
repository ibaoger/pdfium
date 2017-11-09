// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_defaulttypeface.h"

// static
bool CXFA_DefaultTypeface::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_DefaultTypeface::CXFA_DefaultTypeface(CXFA_Document* doc,
                                           XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::DefaultTypeface,
                L"defaultTypeface") {}

CXFA_DefaultTypeface::~CXFA_DefaultTypeface() {}

WideStringView CXFA_DefaultTypeface::GetName() const {
  return L"defaultTypeface";
}

XFA_Element CXFA_DefaultTypeface::GetElementType() const {
  return XFA_Element::DefaultTypeface;
}
