// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_validationmessaging.h"

namespace {

const XFA_Attribute attribute_data[] = {
    XFA_Attribute::Desc, XFA_Attribute::Lock, XFA_Attribute::Unknown};

const wchar_t* kName = L"validationMessaging";

}  // namespace

CXFA_ValidationMessaging::CXFA_ValidationMessaging(CXFA_Document* doc,
                                                   XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Config,
                XFA_ObjectType::NodeV,
                XFA_Element::ValidationMessaging,
                nullptr,
                attribute_data,
                kName) {}

CXFA_ValidationMessaging::~CXFA_ValidationMessaging() {}
