// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_numericedit.h"

// static
bool CXFA_NumericEdit::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_NumericEdit::CXFA_NumericEdit(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::NumericEdit,
                L"numericEdit") {}

CXFA_NumericEdit::~CXFA_NumericEdit() {}

WideStringView CXFA_NumericEdit::GetName() const {
  return L"numericEdit";
}

XFA_Element CXFA_NumericEdit::GetElementType() const {
  return XFA_Element::NumericEdit;
}
