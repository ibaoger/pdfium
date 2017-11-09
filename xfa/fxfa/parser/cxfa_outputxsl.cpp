// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_outputxsl.h"

// static
bool CXFA_OutputXSL::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_OutputXSL::CXFA_OutputXSL(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::OutputXSL,
                L"outputXSL") {}

CXFA_OutputXSL::~CXFA_OutputXSL() {}

WideStringView CXFA_OutputXSL::GetName() const {
  return L"outputXSL";
}

XFA_Element CXFA_OutputXSL::GetElementType() const {
  return XFA_Element::OutputXSL;
}
