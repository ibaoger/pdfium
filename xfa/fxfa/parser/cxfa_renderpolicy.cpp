// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_renderpolicy.h"

// static
bool CXFA_RenderPolicy::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_RenderPolicy::CXFA_RenderPolicy(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::RenderPolicy,
                L"renderPolicy") {}

CXFA_RenderPolicy::~CXFA_RenderPolicy() {}

WideStringView CXFA_RenderPolicy::GetName() const {
  return L"renderPolicy";
}

XFA_Element CXFA_RenderPolicy::GetElementType() const {
  return XFA_Element::RenderPolicy;
}
