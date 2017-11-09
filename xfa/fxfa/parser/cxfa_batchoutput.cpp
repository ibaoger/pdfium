// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_batchoutput.h"

// static
bool CXFA_BatchOutput::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_BatchOutput::CXFA_BatchOutput(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::BatchOutput,
                L"batchOutput") {}

CXFA_BatchOutput::~CXFA_BatchOutput() {}

WideStringView CXFA_BatchOutput::GetName() const {
  return L"batchOutput";
}

XFA_Element CXFA_BatchOutput::GetElementType() const {
  return XFA_Element::BatchOutput;
}
