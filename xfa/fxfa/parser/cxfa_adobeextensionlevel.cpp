// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_adobeextensionlevel.h"

// static
bool CXFA_AdobeExtensionLevel::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_AdobeExtensionLevel::CXFA_AdobeExtensionLevel(CXFA_Document* doc,
                                                   XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::AdobeExtensionLevel,
                L"adobeExtensionLevel") {}

CXFA_AdobeExtensionLevel::~CXFA_AdobeExtensionLevel() {}

WideStringView CXFA_AdobeExtensionLevel::GetName() const {
  return L"adobeExtensionLevel";
}

XFA_Element CXFA_AdobeExtensionLevel::GetElementType() const {
  return XFA_Element::AdobeExtensionLevel;
}
