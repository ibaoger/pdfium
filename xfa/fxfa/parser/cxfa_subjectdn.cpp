// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_subjectdn.h"

// static
bool CXFA_SubjectDN::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_SubjectDN::CXFA_SubjectDN(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeC,
                XFA_Element::SubjectDN,
                L"subjectDN") {}

CXFA_SubjectDN::~CXFA_SubjectDN() {}

WideStringView CXFA_SubjectDN::GetName() const {
  return L"subjectDN";
}

XFA_Element CXFA_SubjectDN::GetElementType() const {
  return XFA_Element::SubjectDN;
}
