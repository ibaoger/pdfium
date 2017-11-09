// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_startpage.h"

// static
bool CXFA_StartPage::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_StartPage::CXFA_StartPage(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::StartPage,
                L"startPage") {}

CXFA_StartPage::~CXFA_StartPage() {}

WideStringView CXFA_StartPage::GetName() const {
  return L"startPage";
}

XFA_Element CXFA_StartPage::GetElementType() const {
  return XFA_Element::StartPage;
}
