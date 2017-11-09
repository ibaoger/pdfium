// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_autosave.h"

// static
bool CXFA_AutoSave::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_AutoSave::CXFA_AutoSave(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::AutoSave,
                L"autoSave") {}

CXFA_AutoSave::~CXFA_AutoSave() {}

WideStringView CXFA_AutoSave::GetName() const {
  return L"autoSave";
}

XFA_Element CXFA_AutoSave::GetElementType() const {
  return XFA_Element::AutoSave;
}
