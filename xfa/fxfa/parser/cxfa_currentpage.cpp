// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_currentpage.h"

// static
bool CXFA_CurrentPage::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_CurrentPage::CXFA_CurrentPage(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::CurrentPage,
                L"currentPage") {}

CXFA_CurrentPage::~CXFA_CurrentPage() {}

WideStringView CXFA_CurrentPage::GetName() const {
  return L"currentPage";
}

XFA_Element CXFA_CurrentPage::GetElementType() const {
  return XFA_Element::CurrentPage;
}
