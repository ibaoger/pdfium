// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_connect.h"

// static
bool CXFA_Connect::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(
      (XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) &
      packet);
}

CXFA_Connect::CXFA_Connect(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Connect,
                L"connect") {}

CXFA_Connect::~CXFA_Connect() {}

WideStringView CXFA_Connect::GetName() const {
  return L"connect";
}

XFA_Element CXFA_Connect::GetElementType() const {
  return XFA_Element::Connect;
}
