// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_format.h"

// static
bool CXFA_Format::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Format::CXFA_Format(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Format,
                L"format") {}

CXFA_Format::~CXFA_Format() {}

WideStringView CXFA_Format::GetName() const {
  return L"format";
}

XFA_Element CXFA_Format::GetElementType() const {
  return XFA_Element::Format;
}
