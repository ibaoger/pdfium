// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_exdata.h"

// static
bool CXFA_ExData::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_ExData::CXFA_ExData(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::ExData,
                L"exData") {}

CXFA_ExData::~CXFA_ExData() {}

WideStringView CXFA_ExData::GetName() const {
  return L"exData";
}

XFA_Element CXFA_ExData::GetElementType() const {
  return XFA_Element::ExData;
}
