// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_dynamicrender.h"

// static
bool CXFA_DynamicRender::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_DynamicRender::CXFA_DynamicRender(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::DynamicRender,
                L"dynamicRender") {}

CXFA_DynamicRender::~CXFA_DynamicRender() {}

WideStringView CXFA_DynamicRender::GetName() const {
  return L"dynamicRender";
}

XFA_Element CXFA_DynamicRender::GetElementType() const {
  return XFA_Element::DynamicRender;
}
