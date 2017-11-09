// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_mediuminfo.h"

// static
bool CXFA_MediumInfo::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_MediumInfo::CXFA_MediumInfo(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::MediumInfo,
                L"mediumInfo") {}

CXFA_MediumInfo::~CXFA_MediumInfo() {}

WideStringView CXFA_MediumInfo::GetName() const {
  return L"mediumInfo";
}

XFA_Element CXFA_MediumInfo::GetElementType() const {
  return XFA_Element::MediumInfo;
}
