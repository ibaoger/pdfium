// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_acrobat7.h"

// static
bool CXFA_Acrobat7::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Acrobat7::CXFA_Acrobat7(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Acrobat7,
                L"acrobat7") {}

CXFA_Acrobat7::~CXFA_Acrobat7() {}

WideStringView CXFA_Acrobat7::GetName() const {
  return L"acrobat7";
}

XFA_Element CXFA_Acrobat7::GetElementType() const {
  return XFA_Element::Acrobat7;
}
