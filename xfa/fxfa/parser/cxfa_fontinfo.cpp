// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_fontinfo.h"

// static
bool CXFA_FontInfo::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_FontInfo::CXFA_FontInfo(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::FontInfo,
                L"fontInfo") {}

CXFA_FontInfo::~CXFA_FontInfo() {}

WideStringView CXFA_FontInfo::GetName() const {
  return L"fontInfo";
}

XFA_Element CXFA_FontInfo::GetElementType() const {
  return XFA_Element::FontInfo;
}
