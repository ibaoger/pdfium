// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_printhighquality.h"

// static
bool CXFA_PrintHighQuality::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_PrintHighQuality::CXFA_PrintHighQuality(CXFA_Document* doc,
                                             XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::PrintHighQuality,
                L"printHighQuality") {}

CXFA_PrintHighQuality::~CXFA_PrintHighQuality() {}

WideStringView CXFA_PrintHighQuality::GetName() const {
  return L"printHighQuality";
}

XFA_Element CXFA_PrintHighQuality::GetElementType() const {
  return XFA_Element::PrintHighQuality;
}
