// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_acrobat7.h"

namespace {

const CXFA_Node::PropertyData property_data[] = {
    {XFA_Element::DynamicRender, 1, 0},
    {XFA_Element::Unknown, 0, 0}};

const XFA_Attribute attribute_data[] = {
    XFA_Attribute::Desc, XFA_Attribute::Lock, XFA_Attribute::Unknown};

const wchar_t* kName = L"acrobat7";

}  // namespace

CXFA_Acrobat7::CXFA_Acrobat7(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Config,
                XFA_ObjectType::Node,
                XFA_Element::Acrobat7,
                property_data,
                attribute_data,
                kName) {}

CXFA_Acrobat7::~CXFA_Acrobat7() {}
