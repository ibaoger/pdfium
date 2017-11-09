// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_instancemanager.h"

// static
bool CXFA_InstanceManager::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Form & packet);
}

CXFA_InstanceManager::CXFA_InstanceManager(CXFA_Document* doc,
                                           XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::InstanceManager,
                L"instanceManager") {}

CXFA_InstanceManager::~CXFA_InstanceManager() {}

WideStringView CXFA_InstanceManager::GetName() const {
  return L"instanceManager";
}

XFA_Element CXFA_InstanceManager::GetElementType() const {
  return XFA_Element::InstanceManager;
}
