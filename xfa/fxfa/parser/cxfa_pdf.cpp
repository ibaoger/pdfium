// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pdf.h"

// static
bool CXFA_Pdf::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Pdf::CXFA_Pdf(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Pdf, L"pdf") {}

CXFA_Pdf::~CXFA_Pdf() {}

WideStringView CXFA_Pdf::GetName() const {
  return L"pdf";
}

XFA_Element CXFA_Pdf::GetElementType() const {
  return XFA_Element::Pdf;
}
