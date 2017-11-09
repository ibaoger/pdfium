// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_manifest.h"

// static
bool CXFA_Manifest::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) & packet);
}

CXFA_Manifest::CXFA_Manifest(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Manifest,
                L"manifest") {}

CXFA_Manifest::~CXFA_Manifest() {}

WideStringView CXFA_Manifest::GetName() const {
  return L"manifest";
}

XFA_Element CXFA_Manifest::GetElementType() const {
  return XFA_Element::Manifest;
}
