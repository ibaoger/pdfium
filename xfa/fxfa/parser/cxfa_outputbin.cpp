// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_outputbin.h"

// static
bool CXFA_OutputBin::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_OutputBin::CXFA_OutputBin(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::OutputBin,
                L"outputBin") {}

CXFA_OutputBin::~CXFA_OutputBin() {}

WideStringView CXFA_OutputBin::GetName() const {
  return L"outputBin";
}

XFA_Element CXFA_OutputBin::GetElementType() const {
  return XFA_Element::OutputBin;
}
