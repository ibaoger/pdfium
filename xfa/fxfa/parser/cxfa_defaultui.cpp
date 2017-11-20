// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_defaultui.h"

namespace {

const CXFA_Node::PropertyData property_data[] = {{XFA_Element::Extras, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute attribute_data[] = {XFA_Attribute::Id, XFA_Attribute::Use,
                                        XFA_Attribute::Usehref,
                                        XFA_Attribute::Unknown};

const wchar_t* kName = L"defaultUi";

}  // namespace

CXFA_DefaultUi::CXFA_DefaultUi(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::DefaultUi,
                property_data,
                attribute_data,
                kName) {}

CXFA_DefaultUi::~CXFA_DefaultUi() {}
