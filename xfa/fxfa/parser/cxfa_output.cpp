// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_output.h"

// static
bool CXFA_Output::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Output::CXFA_Output(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Output,
                L"output") {}

CXFA_Output::~CXFA_Output() {}

WideStringView CXFA_Output::GetName() const {
  return L"output";
}

XFA_Element CXFA_Output::GetElementType() const {
  return XFA_Element::Output;
}
