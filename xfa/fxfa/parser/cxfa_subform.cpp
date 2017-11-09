// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_subform.h"

// static
bool CXFA_Subform::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Subform::CXFA_Subform(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContainerNode,
                XFA_Element::Subform,
                L"subform") {}

CXFA_Subform::~CXFA_Subform() {}

WideStringView CXFA_Subform::GetName() const {
  return L"subform";
}

XFA_Element CXFA_Subform::GetElementType() const {
  return XFA_Element::Subform;
}
