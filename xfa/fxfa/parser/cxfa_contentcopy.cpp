// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_contentcopy.h"

// static
bool CXFA_ContentCopy::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_ContentCopy::CXFA_ContentCopy(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::ContentCopy,
                L"contentCopy") {}

CXFA_ContentCopy::~CXFA_ContentCopy() {}

WideStringView CXFA_ContentCopy::GetName() const {
  return L"contentCopy";
}

XFA_Element CXFA_ContentCopy::GetElementType() const {
  return XFA_Element::ContentCopy;
}
