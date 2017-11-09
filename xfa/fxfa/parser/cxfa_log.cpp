// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_log.h"

// static
bool CXFA_Log::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Log::CXFA_Log(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc, packet, XFA_ObjectType::Node, XFA_Element::Log, L"log") {}

CXFA_Log::~CXFA_Log() {}

WideStringView CXFA_Log::GetName() const {
  return L"log";
}

XFA_Element CXFA_Log::GetElementType() const {
  return XFA_Element::Log;
}
