// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_versioncontrol.h"

// static
bool CXFA_VersionControl::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_VersionControl::CXFA_VersionControl(CXFA_Document* doc,
                                         XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::VersionControl,
                L"versionControl") {}

CXFA_VersionControl::~CXFA_VersionControl() {}

WideStringView CXFA_VersionControl::GetName() const {
  return L"versionControl";
}

XFA_Element CXFA_VersionControl::GetElementType() const {
  return XFA_Element::VersionControl;
}
