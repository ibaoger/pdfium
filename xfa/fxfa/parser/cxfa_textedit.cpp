// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_textedit.h"

// static
bool CXFA_TextEdit::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_TextEdit::CXFA_TextEdit(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::TextEdit,
                L"textEdit") {}

CXFA_TextEdit::~CXFA_TextEdit() {}

WideStringView CXFA_TextEdit::GetName() const {
  return L"textEdit";
}

XFA_Element CXFA_TextEdit::GetElementType() const {
  return XFA_Element::TextEdit;
}
