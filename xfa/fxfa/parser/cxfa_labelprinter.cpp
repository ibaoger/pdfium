// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_labelprinter.h"

namespace {

const CXFA_Node::PropertyData property_data[] = {
    {XFA_Element::FontInfo, 1, 0},
    {XFA_Element::Xdc, 1, 0},
    {XFA_Element::BatchOutput, 1, 0},
    {XFA_Element::FlipLabel, 1, 0},
    {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute attribute_data[] = {
    XFA_Attribute::Name, XFA_Attribute::Desc, XFA_Attribute::Lock,
    XFA_Attribute::Unknown};

const wchar_t* kName = L"labelPrinter";

}  // namespace

CXFA_LabelPrinter::CXFA_LabelPrinter(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Config,
                XFA_ObjectType::Node,
                XFA_Element::LabelPrinter,
                property_data,
                attribute_data,
                kName) {}

CXFA_LabelPrinter::~CXFA_LabelPrinter() {}
