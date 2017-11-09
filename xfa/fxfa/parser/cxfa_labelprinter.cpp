// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_labelprinter.h"

// static
bool CXFA_LabelPrinter::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_LabelPrinter::CXFA_LabelPrinter(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::LabelPrinter,
                L"labelPrinter") {}

CXFA_LabelPrinter::~CXFA_LabelPrinter() {}

WideStringView CXFA_LabelPrinter::GetName() const {
  return L"labelPrinter";
}

XFA_Element CXFA_LabelPrinter::GetElementType() const {
  return XFA_Element::LabelPrinter;
}
