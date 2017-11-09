// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pcl.h"

// static
bool CXFA_Pcl::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Pcl::CXFA_Pcl(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Pcl, L"pcl") {}

CXFA_Pcl::~CXFA_Pcl() {}

WideStringView CXFA_Pcl::GetName() const {
  return L"pcl";
}

XFA_Element CXFA_Pcl::GetElementType() const {
  return XFA_Element::Pcl;
}
