// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_printername.h"

// static
bool CXFA_PrinterName::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_PrinterName::CXFA_PrinterName(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::PrinterName,
                L"printerName") {}

CXFA_PrinterName::~CXFA_PrinterName() {}

WideStringView CXFA_PrinterName::GetName() const {
  return L"printerName";
}

XFA_Element CXFA_PrinterName::GetElementType() const {
  return XFA_Element::PrinterName;
}
