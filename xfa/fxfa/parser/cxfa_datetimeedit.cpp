// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_datetimeedit.h"

namespace {

const CXFA_Node::PropertyData property_data[] = {{XFA_Element::Margin, 1, 0},
                                                 {XFA_Element::Border, 1, 0},
                                                 {XFA_Element::Comb, 1, 0},
                                                 {XFA_Element::Extras, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};

const XFA_Attribute attribute_data[] = {XFA_Attribute::Id,
                                        XFA_Attribute::Use,
                                        XFA_Attribute::Usehref,
                                        XFA_Attribute::Picker,
                                        XFA_Attribute::HScrollPolicy,
                                        XFA_Attribute::Unknown};

const wchar_t* kName = L"dateTimeEdit";

}  // namespace

CXFA_DateTimeEdit::CXFA_DateTimeEdit(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::DateTimeEdit,
                property_data,
                attribute_data,
                kName) {}

CXFA_DateTimeEdit::~CXFA_DateTimeEdit() {}
