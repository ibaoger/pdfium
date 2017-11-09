// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_command.h"

// static
bool CXFA_Command::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_SourceSet & packet);
}

CXFA_Command::CXFA_Command(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Command,
                L"command") {}

CXFA_Command::~CXFA_Command() {}

WideStringView CXFA_Command::GetName() const {
  return L"command";
}

XFA_Element CXFA_Command::GetElementType() const {
  return XFA_Element::Command;
}
