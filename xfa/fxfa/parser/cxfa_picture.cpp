// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_picture.h"

// static
bool CXFA_Picture::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(
      (XFA_XDPPACKET_Config | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form) &
      packet);
}

CXFA_Picture::CXFA_Picture(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Picture,
                L"picture") {}

CXFA_Picture::~CXFA_Picture() {}

WideStringView CXFA_Picture::GetName() const {
  return L"picture";
}

XFA_Element CXFA_Picture::GetElementType() const {
  return XFA_Element::Picture;
}
