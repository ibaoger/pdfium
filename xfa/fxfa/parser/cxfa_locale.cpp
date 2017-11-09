// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_locale.h"

// static
bool CXFA_Locale::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Config | XFA_XDPPACKET_LocaleSet) & packet);
}

CXFA_Locale::CXFA_Locale(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::Locale,
                L"locale") {}

CXFA_Locale::~CXFA_Locale() {}

WideStringView CXFA_Locale::GetName() const {
  return L"locale";
}

XFA_Element CXFA_Locale::GetElementType() const {
  return XFA_Element::Locale;
}
