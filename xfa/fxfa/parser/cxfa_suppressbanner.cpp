// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_suppressbanner.h"

// static
bool CXFA_SuppressBanner::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_SuppressBanner::CXFA_SuppressBanner(CXFA_Document* doc,
                                         XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::SuppressBanner,
                L"suppressBanner") {}

CXFA_SuppressBanner::~CXFA_SuppressBanner() {}

WideStringView CXFA_SuppressBanner::GetName() const {
  return L"suppressBanner";
}

XFA_Element CXFA_SuppressBanner::GetElementType() const {
  return XFA_Element::SuppressBanner;
}
