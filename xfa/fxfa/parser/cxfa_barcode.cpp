// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_barcode.h"

// static
bool CXFA_Barcode::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Barcode::CXFA_Barcode(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Barcode,
                L"barcode") {}

CXFA_Barcode::~CXFA_Barcode() {}

WideStringView CXFA_Barcode::GetName() const {
  return L"barcode";
}

XFA_Element CXFA_Barcode::GetElementType() const {
  return XFA_Element::Barcode;
}
