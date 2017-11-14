// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pdf.h"

namespace {

const CXFA_Node::PropertyData property_data[] = {
    {XFA_Element::AdobeExtensionLevel, 1, 0},
    {XFA_Element::FontInfo, 1, 0},
    {XFA_Element::Xdc, 1, 0},
    {XFA_Element::Pdfa, 1, 0},
    {XFA_Element::BatchOutput, 1, 0},
    {XFA_Element::ViewerPreferences, 1, 0},
    {XFA_Element::ScriptModel, 1, 0},
    {XFA_Element::Version, 1, 0},
    {XFA_Element::SubmitFormat, 1, 0},
    {XFA_Element::SilentPrint, 1, 0},
    {XFA_Element::Producer, 1, 0},
    {XFA_Element::Compression, 1, 0},
    {XFA_Element::Interactive, 1, 0},
    {XFA_Element::Encryption, 1, 0},
    {XFA_Element::RenderPolicy, 1, 0},
    {XFA_Element::OpenAction, 1, 0},
    {XFA_Element::Creator, 1, 0},
    {XFA_Element::Linearized, 1, 0},
    {XFA_Element::Tagged, 1, 0},
    {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute attribute_data[] = {
    XFA_Attribute::Name, XFA_Attribute::Desc, XFA_Attribute::Lock,
    XFA_Attribute::Unknown};

const wchar_t* kName = L"pdf";

}  // namespace

CXFA_Pdf::CXFA_Pdf(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Config,
                XFA_ObjectType::Node,
                XFA_Element::Pdf,
                property_data,
                attribute_data,
                kName) {}

CXFA_Pdf::~CXFA_Pdf() {}
