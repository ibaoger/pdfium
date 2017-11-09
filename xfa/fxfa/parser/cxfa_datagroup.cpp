// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_datagroup.h"

// static
bool CXFA_DataGroup::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Datasets & packet);
}

CXFA_DataGroup::CXFA_DataGroup(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::DataGroup,
                L"dataGroup") {}

CXFA_DataGroup::~CXFA_DataGroup() {}

WideStringView CXFA_DataGroup::GetName() const {
  return L"dataGroup";
}

XFA_Element CXFA_DataGroup::GetElementType() const {
  return XFA_Element::DataGroup;
}
