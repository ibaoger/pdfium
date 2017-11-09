// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_bookend.h"

// static
bool CXFA_Bookend::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Bookend::CXFA_Bookend(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Bookend,
                L"bookend") {}

CXFA_Bookend::~CXFA_Bookend() {}

WideStringView CXFA_Bookend::GetName() const {
  return L"bookend";
}

XFA_Element CXFA_Bookend::GetElementType() const {
  return XFA_Element::Bookend;
}
