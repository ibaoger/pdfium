// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_viewerpreferences.h"

// static
bool CXFA_ViewerPreferences::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_ViewerPreferences::CXFA_ViewerPreferences(CXFA_Document* doc,
                                               XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::ViewerPreferences,
                L"viewerPreferences") {}

CXFA_ViewerPreferences::~CXFA_ViewerPreferences() {}

WideStringView CXFA_ViewerPreferences::GetName() const {
  return L"viewerPreferences";
}

XFA_Element CXFA_ViewerPreferences::GetElementType() const {
  return XFA_Element::ViewerPreferences;
}
