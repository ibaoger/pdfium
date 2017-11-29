// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pageoffset.h"

namespace {

const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::X, XFA_AttributeType::Measure,
     XFA_XDPPACKET_Config | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form,
     (void*)L"0in"},
    {XFA_Attribute::Y, XFA_AttributeType::Measure,
     XFA_XDPPACKET_Config | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form,
     (void*)L"0in"},
    {XFA_Attribute::Desc, XFA_AttributeType::CData,
     XFA_XDPPACKET_Config | XFA_XDPPACKET_LocaleSet, nullptr},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, XFA_XDPPACKET_Config,
     (void*)0},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, 0, nullptr}};

constexpr wchar_t kName[] = L"pageOffset";

}  // namespace

CXFA_PageOffset::CXFA_PageOffset(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Config,
                XFA_ObjectType::Node,
                XFA_Element::PageOffset,
                nullptr,
                kAttributeData,
                kName) {}

CXFA_PageOffset::~CXFA_PageOffset() {}
