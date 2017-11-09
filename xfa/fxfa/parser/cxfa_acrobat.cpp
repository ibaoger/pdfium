// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_acrobat.h"

// static
bool CXFA_Acrobat::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Acrobat::CXFA_Acrobat(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Acrobat,
                L"acrobat") {}

CXFA_Acrobat::~CXFA_Acrobat() {}

WideStringView CXFA_Acrobat::GetName() const {
  return L"acrobat";
}

XFA_Element CXFA_Acrobat::GetElementType() const {
  return XFA_Element::Acrobat;
}
