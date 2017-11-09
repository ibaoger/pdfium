// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_template.h"

// static
bool CXFA_Template::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(
      (XFA_XDPPACKET_Config | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) &
      packet);
}

CXFA_Template::CXFA_Template(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ModelNode,
                XFA_Element::Template,
                L"template") {}

CXFA_Template::~CXFA_Template() {}

WideStringView CXFA_Template::GetName() const {
  return L"template";
}

XFA_Element CXFA_Template::GetElementType() const {
  return XFA_Element::Template;
}
