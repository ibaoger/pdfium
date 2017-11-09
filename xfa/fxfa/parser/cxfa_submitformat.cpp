// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_submitformat.h"

// static
bool CXFA_SubmitFormat::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_SubmitFormat::CXFA_SubmitFormat(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::SubmitFormat,
                L"submitFormat") {}

CXFA_SubmitFormat::~CXFA_SubmitFormat() {}

WideStringView CXFA_SubmitFormat::GetName() const {
  return L"submitFormat";
}

XFA_Element CXFA_SubmitFormat::GetElementType() const {
  return XFA_Element::SubmitFormat;
}
