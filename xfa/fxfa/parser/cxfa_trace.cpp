// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_trace.h"

// static
bool CXFA_Trace::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Trace::CXFA_Trace(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Trace,
                L"trace") {}

CXFA_Trace::~CXFA_Trace() {}

WideStringView CXFA_Trace::GetName() const {
  return L"trace";
}

XFA_Element CXFA_Trace::GetElementType() const {
  return XFA_Element::Trace;
}
