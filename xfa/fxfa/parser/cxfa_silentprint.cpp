// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_silentprint.h"

// static
bool CXFA_SilentPrint::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_SilentPrint::CXFA_SilentPrint(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::SilentPrint,
                L"silentPrint") {}

CXFA_SilentPrint::~CXFA_SilentPrint() {}

WideStringView CXFA_SilentPrint::GetName() const {
  return L"silentPrint";
}

XFA_Element CXFA_SilentPrint::GetElementType() const {
  return XFA_Element::SilentPrint;
}
