// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_config.h"

// static
bool CXFA_Config::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Config::CXFA_Config(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ModelNode,
                XFA_Element::Config,
                L"config") {}

CXFA_Config::~CXFA_Config() {}

WideStringView CXFA_Config::GetName() const {
  return L"config";
}

XFA_Element CXFA_Config::GetElementType() const {
  return XFA_Element::Config;
}
