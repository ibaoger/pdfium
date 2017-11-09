// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_addviewerpreferences.h"

// static
bool CXFA_AddViewerPreferences::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_AddViewerPreferences::CXFA_AddViewerPreferences(CXFA_Document* doc,
                                                     XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ContentNode,
                XFA_Element::AddViewerPreferences,
                L"addViewerPreferences") {}

CXFA_AddViewerPreferences::~CXFA_AddViewerPreferences() {}

WideStringView CXFA_AddViewerPreferences::GetName() const {
  return L"addViewerPreferences";
}

XFA_Element CXFA_AddViewerPreferences::GetElementType() const {
  return XFA_Element::AddViewerPreferences;
}
