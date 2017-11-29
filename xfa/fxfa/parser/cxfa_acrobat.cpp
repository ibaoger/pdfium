// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_acrobat.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::AutoSave, 1, 0},
    {XFA_Element::Validate, 1, 0},
    {XFA_Element::ValidateApprovalSignatures, 1, 0},
    {XFA_Element::Acrobat7, 1, 0},
    {XFA_Element::Common, 1, 0},
    {XFA_Element::Unknown, 0, 0}};
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Desc, XFA_AttributeType::CData,
     XFA_XDPPACKET_Config | XFA_XDPPACKET_LocaleSet, nullptr},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, XFA_XDPPACKET_Config,
     (void*)0},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, 0, nullptr}};

constexpr wchar_t kName[] = L"acrobat";

}  // namespace

CXFA_Acrobat::CXFA_Acrobat(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Config,
                XFA_ObjectType::Node,
                XFA_Element::Acrobat,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Acrobat::~CXFA_Acrobat() {}
