// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_filter.h"

// static
bool CXFA_Filter::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Filter::CXFA_Filter(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Filter,
                L"filter") {}

CXFA_Filter::~CXFA_Filter() {}

WideStringView CXFA_Filter::GetName() const {
  return L"filter";
}

XFA_Element CXFA_Filter::GetElementType() const {
  return XFA_Element::Filter;
}
