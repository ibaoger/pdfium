// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_version.h"

// static
bool CXFA_Version::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Version::CXFA_Version(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Version,
                L"version") {}

CXFA_Version::~CXFA_Version() {}

WideStringView CXFA_Version::GetName() const {
  return L"version";
}

XFA_Element CXFA_Version::GetElementType() const {
  return XFA_Element::Version;
}
