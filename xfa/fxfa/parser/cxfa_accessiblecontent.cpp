// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_accessiblecontent.h"

// static
bool CXFA_AccessibleContent::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_AccessibleContent::CXFA_AccessibleContent(CXFA_Document* doc,
                                               XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::AccessibleContent,
                L"accessibleContent") {}

CXFA_AccessibleContent::~CXFA_AccessibleContent() {}

WideStringView CXFA_AccessibleContent::GetName() const {
  return L"accessibleContent";
}

XFA_Element CXFA_AccessibleContent::GetElementType() const {
  return XFA_Element::AccessibleContent;
}
