// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_xsdconnection.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::Uri, 1, 0},
    {XFA_Element::RootElement, 1, 0},
    {XFA_Element::Unknown, 0, 0}};
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Name, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Config | XFA_XDPPACKET_LocaleSet |
         XFA_XDPPACKET_Template | XFA_XDPPACKET_Datasets | XFA_XDPPACKET_Form |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::DataDescription, XFA_AttributeType::CData,
     XFA_XDPPACKET_ConnectionSet, nullptr},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, 0, nullptr}};

constexpr wchar_t kName[] = L"xsdConnection";

}  // namespace

CXFA_XsdConnection::CXFA_XsdConnection(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_ConnectionSet,
                XFA_ObjectType::Node,
                XFA_Element::XsdConnection,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_XsdConnection::~CXFA_XsdConnection() {}
