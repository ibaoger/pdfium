// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_imageedit.h"

// static
bool CXFA_ImageEdit::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_ImageEdit::CXFA_ImageEdit(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::ImageEdit,
                L"imageEdit") {}

CXFA_ImageEdit::~CXFA_ImageEdit() {}

WideStringView CXFA_ImageEdit::GetName() const {
  return L"imageEdit";
}

XFA_Element CXFA_ImageEdit::GetElementType() const {
  return XFA_Element::ImageEdit;
}
