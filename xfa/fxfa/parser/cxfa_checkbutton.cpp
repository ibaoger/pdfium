// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_checkbutton.h"

namespace {

const CXFA_Node::PropertyData property_data[] = {{XFA_Element::Margin, 1, 0},
                                                 {XFA_Element::Border, 1, 0},
                                                 {XFA_Element::Extras, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute attribute_data[] = {
    XFA_Attribute::Id,      XFA_Attribute::Use,    XFA_Attribute::AllowNeutral,
    XFA_Attribute::Mark,    XFA_Attribute::Shape,  XFA_Attribute::Size,
    XFA_Attribute::Usehref, XFA_Attribute::Unknown};

const wchar_t* kName = L"checkButton";

}  // namespace

CXFA_CheckButton::CXFA_CheckButton(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::CheckButton,
                property_data,
                attribute_data,
                kName) {}

CXFA_CheckButton::~CXFA_CheckButton() {}
