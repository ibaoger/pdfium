// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_subjectdns.h"

// static
bool CXFA_SubjectDNs::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_SubjectDNs::CXFA_SubjectDNs(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::SubjectDNs,
                L"subjectDNs") {}

CXFA_SubjectDNs::~CXFA_SubjectDNs() {}

WideStringView CXFA_SubjectDNs::GetName() const {
  return L"subjectDNs";
}

XFA_Element CXFA_SubjectDNs::GetElementType() const {
  return XFA_Element::SubjectDNs;
}
