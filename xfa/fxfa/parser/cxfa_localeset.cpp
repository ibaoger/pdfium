// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_localeset.h"

// static
bool CXFA_LocaleSet::IsValidPacket(XFA_XDPPACKET packet) {
  return !!((XFA_XDPPACKET_Config | XFA_XDPPACKET_LocaleSet) & packet);
}

CXFA_LocaleSet::CXFA_LocaleSet(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::ModelNode,
                XFA_Element::LocaleSet,
                L"localeSet") {}

CXFA_LocaleSet::~CXFA_LocaleSet() {}

WideStringView CXFA_LocaleSet::GetName() const {
  return L"localeSet";
}

XFA_Element CXFA_LocaleSet::GetElementType() const {
  return XFA_Element::LocaleSet;
}
