// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_proto.h"

// static
bool CXFA_Proto::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Proto::CXFA_Proto(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Proto,
                L"proto") {}

CXFA_Proto::~CXFA_Proto() {}

WideStringView CXFA_Proto::GetName() const {
  return L"proto";
}

XFA_Element CXFA_Proto::GetElementType() const {
  return XFA_Element::Proto;
}
