// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pageset.h"

// static
bool CXFA_PageSet::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_PageSet::CXFA_PageSet(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContainerNode,
                XFA_Element::PageSet,
                L"pageSet") {}

CXFA_PageSet::~CXFA_PageSet() {}

WideStringView CXFA_PageSet::GetName() const {
  return L"pageSet";
}

XFA_Element CXFA_PageSet::GetElementType() const {
  return XFA_Element::PageSet;
}
