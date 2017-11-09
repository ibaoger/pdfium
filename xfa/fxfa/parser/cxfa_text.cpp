// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_text.h"

// static
bool CXFA_Text::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(
      (XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) &
      packet);
}

CXFA_Text::CXFA_Text(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Text,
                L"text") {}

CXFA_Text::~CXFA_Text() {}

WideStringView CXFA_Text::GetName() const {
  return L"text";
}

XFA_Element CXFA_Text::GetElementType() const {
  return XFA_Element::Text;
}
