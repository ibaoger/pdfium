// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_rectangle.h"

namespace {

const CXFA_Node::PropertyData property_data[] = {{XFA_Element::Edge, 4, 0},
                                                 {XFA_Element::Corner, 4, 0},
                                                 {XFA_Element::Fill, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};

const XFA_Attribute attribute_data[] = {
    XFA_Attribute::Id, XFA_Attribute::Use, XFA_Attribute::Usehref,
    XFA_Attribute::Hand, XFA_Attribute::Unknown};

const wchar_t* kName = L"rectangle";

}  // namespace

CXFA_Rectangle::CXFA_Rectangle(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Rectangle,
                property_data,
                attribute_data,
                kName) {}

CXFA_Rectangle::~CXFA_Rectangle() {}
