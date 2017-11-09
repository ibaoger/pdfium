// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_record.h"

// static
bool CXFA_Record::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Record::CXFA_Record(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::Record,
                L"record") {}

CXFA_Record::~CXFA_Record() {}

WideStringView CXFA_Record::GetName() const {
  return L"record";
}

XFA_Element CXFA_Record::GetElementType() const {
  return XFA_Element::Record;
}
