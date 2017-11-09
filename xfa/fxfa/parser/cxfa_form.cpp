// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_form.h"

// static
bool CXFA_Form::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Form & packet);
}

CXFA_Form::CXFA_Form(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ModelNode,
                XFA_Element::Form,
                L"form") {}

CXFA_Form::~CXFA_Form() {}

WideStringView CXFA_Form::GetName() const {
  return L"form";
}

XFA_Element CXFA_Form::GetElementType() const {
  return XFA_Element::Form;
}
