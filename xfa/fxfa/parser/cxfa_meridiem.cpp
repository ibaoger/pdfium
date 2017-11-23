// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_meridiem.h"

namespace {

constexpr wchar_t kName[] = L"meridiem";

}  // namespace

CXFA_Meridiem::CXFA_Meridiem(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_LocaleSet,
                XFA_ObjectType::ContentNode,
                XFA_Element::Meridiem,
                nullptr,
                nullptr,
                kName) {}

CXFA_Meridiem::~CXFA_Meridiem() {}
