// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_hyphenation.h"

namespace {

const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::WordCharacterCount, XFA_AttributeType::Integer,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form, (void*)7},
    {XFA_Attribute::Hyphenate, XFA_AttributeType::Boolean,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form, (void*)0},
    {XFA_Attribute::ExcludeInitialCap, XFA_AttributeType::Boolean,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form, (void*)0},
    {XFA_Attribute::PushCharacterCount, XFA_AttributeType::Integer,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form, (void*)3},
    {XFA_Attribute::RemainCharacterCount, XFA_AttributeType::Integer,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form, (void*)3},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::ExcludeAllCaps, XFA_AttributeType::Boolean,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form, (void*)0},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, 0, nullptr}};

constexpr wchar_t kName[] = L"hyphenation";

}  // namespace

CXFA_Hyphenation::CXFA_Hyphenation(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Hyphenation,
                nullptr,
                kAttributeData,
                kName) {}

CXFA_Hyphenation::~CXFA_Hyphenation() {}
