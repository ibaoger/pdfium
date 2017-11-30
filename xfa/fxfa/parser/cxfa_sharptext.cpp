// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_sharptext.h"

namespace {

const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Value, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Config | XFA_XDPPACKET_LocaleSet |
         XFA_XDPPACKET_Template | XFA_XDPPACKET_Datasets |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, 0, nullptr}};

constexpr wchar_t kName[] = L"#text";

}  // namespace

CXFA_Sharptext::CXFA_Sharptext(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Config |
                 XFA_XDPPACKET_LocaleSet | XFA_XDPPACKET_ConnectionSet |
                 XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Form),
                XFA_ObjectType::NodeV,
                XFA_Element::Sharptext,
                nullptr,
                kAttributeData,
                kName) {}

CXFA_Sharptext::~CXFA_Sharptext() {}
