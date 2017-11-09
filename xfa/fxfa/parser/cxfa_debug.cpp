// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_debug.h"

// static
bool CXFA_Debug::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Debug::CXFA_Debug(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Debug,
                L"debug") {}

CXFA_Debug::~CXFA_Debug() {}

WideStringView CXFA_Debug::GetName() const {
  return L"debug";
}

XFA_Element CXFA_Debug::GetElementType() const {
  return XFA_Element::Debug;
}
