// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_breakbefore.h"

// static
bool CXFA_BreakBefore::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_BreakBefore::CXFA_BreakBefore(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::BreakBefore,
                L"breakBefore") {}

CXFA_BreakBefore::~CXFA_BreakBefore() {}

WideStringView CXFA_BreakBefore::GetName() const {
  return L"breakBefore";
}

XFA_Element CXFA_BreakBefore::GetElementType() const {
  return XFA_Element::BreakBefore;
}
