// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_typefaces.h"

// static
bool CXFA_Typefaces::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_LocaleSet & packet);
}

CXFA_Typefaces::CXFA_Typefaces(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Typefaces,
                L"typefaces") {}

CXFA_Typefaces::~CXFA_Typefaces() {}

WideStringView CXFA_Typefaces::GetName() const {
  return L"typefaces";
}

XFA_Element CXFA_Typefaces::GetElementType() const {
  return XFA_Element::Typefaces;
}
