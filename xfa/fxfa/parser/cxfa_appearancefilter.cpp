// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_appearancefilter.h"

// static
bool CXFA_AppearanceFilter::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_AppearanceFilter::CXFA_AppearanceFilter(CXFA_Document* doc,
                                             XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeC,
                XFA_Element::AppearanceFilter,
                L"appearanceFilter") {}

CXFA_AppearanceFilter::~CXFA_AppearanceFilter() {}

WideStringView CXFA_AppearanceFilter::GetName() const {
  return L"appearanceFilter";
}

XFA_Element CXFA_AppearanceFilter::GetElementType() const {
  return XFA_Element::AppearanceFilter;
}
