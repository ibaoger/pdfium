// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_setproperty.h"

namespace {

const XFA_Attribute attribute_data[] = {
    XFA_Attribute::Ref, XFA_Attribute::Connection, XFA_Attribute::Target,
    XFA_Attribute::Unknown};

const wchar_t* kName = L"setProperty";

}  // namespace

CXFA_SetProperty::CXFA_SetProperty(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::SetProperty,
                nullptr,
                attribute_data,
                kName) {}

CXFA_SetProperty::~CXFA_SetProperty() {}
