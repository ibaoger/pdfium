// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_uri.h"

// static
bool CXFA_Uri::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Config | XFA_XDPPACKET_ConnectionSet) & packet);
}

CXFA_Uri::CXFA_Uri(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::TextNode,
                XFA_Element::Uri,
                L"uri") {}

CXFA_Uri::~CXFA_Uri() {}

WideStringView CXFA_Uri::GetName() const {
  return L"uri";
}

XFA_Element CXFA_Uri::GetElementType() const {
  return XFA_Element::Uri;
}
