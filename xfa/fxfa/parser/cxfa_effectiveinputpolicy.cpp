// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_effectiveinputpolicy.h"

// static
bool CXFA_EffectiveInputPolicy::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_ConnectionSet & packet);
}

CXFA_EffectiveInputPolicy::CXFA_EffectiveInputPolicy(CXFA_Document* doc,
                                                     XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::EffectiveInputPolicy,
                L"effectiveInputPolicy") {}

CXFA_EffectiveInputPolicy::~CXFA_EffectiveInputPolicy() {}

WideStringView CXFA_EffectiveInputPolicy::GetName() const {
  return L"effectiveInputPolicy";
}

XFA_Element CXFA_EffectiveInputPolicy::GetElementType() const {
  return XFA_Element::EffectiveInputPolicy;
}
