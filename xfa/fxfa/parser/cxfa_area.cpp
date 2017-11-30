// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_area.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {{XFA_Element::Desc, 1, 0},
                                                 {XFA_Element::Extras, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::X, XFA_AttributeType::Measure,
     XFA_XDPPACKET_Config | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form,
     (void*)L"0in"},
    {XFA_Attribute::Y, XFA_AttributeType::Measure,
     XFA_XDPPACKET_Config | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form,
     (void*)L"0in"},
    {XFA_Attribute::Id, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Config | XFA_XDPPACKET_LocaleSet |
         XFA_XDPPACKET_Template | XFA_XDPPACKET_Datasets | XFA_XDPPACKET_Form |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Level, XFA_AttributeType::Integer, XFA_XDPPACKET_Config,
     (void*)0},
    {XFA_Attribute::Relevant, XFA_AttributeType::CData,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form, nullptr},
    {XFA_Attribute::ColSpan, XFA_AttributeType::Integer,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form, (void*)1},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, XFA_XDPPACKET_Config,
     (void*)0},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, 0, nullptr}};

constexpr wchar_t kName[] = L"area";

}  // namespace

CXFA_Area::CXFA_Area(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(
          doc,
          packet,
          (XFA_XDPPACKET_Config | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
          XFA_ObjectType::ContainerNode,
          XFA_Element::Area,
          kPropertyData,
          kAttributeData,
          kName) {}

CXFA_Area::~CXFA_Area() {}
