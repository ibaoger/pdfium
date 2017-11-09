// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_script.h"

// static
bool CXFA_Script::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(
      (XFA_XDPPACKET_Config | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) &
      packet);
}

CXFA_Script::CXFA_Script(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Script,
                L"script") {}

CXFA_Script::~CXFA_Script() {}

WideStringView CXFA_Script::GetName() const {
  return L"script";
}

XFA_Element CXFA_Script::GetElementType() const {
  return XFA_Element::Script;
}
