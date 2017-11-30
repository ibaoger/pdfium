// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_binditems.h"

namespace {

const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Ref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Connection, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::LabelRef, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::ValueRef, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, nullptr}};

constexpr wchar_t kName[] = L"bindItems";

}  // namespace

CXFA_BindItems::CXFA_BindItems(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::BindItems,
                nullptr,
                kAttributeData,
                kName) {}

CXFA_BindItems::~CXFA_BindItems() {}
