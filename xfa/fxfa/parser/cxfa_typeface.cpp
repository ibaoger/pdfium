// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_typeface.h"

// static
bool CXFA_Typeface::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_Typeface::CXFA_Typeface(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Typeface,
                L"typeface") {}

CXFA_Typeface::~CXFA_Typeface() {}

WideStringView CXFA_Typeface::GetName() const {
  return L"typeface";
}

XFA_Element CXFA_Typeface::GetElementType() const {
  return XFA_Element::Typeface;
}
