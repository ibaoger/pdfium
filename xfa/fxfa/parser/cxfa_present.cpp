// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_present.h"

namespace {

const CXFA_Node::PropertyData property_data[] = {
    {XFA_Element::Xdp, 1, 0},
    {XFA_Element::Cache, 1, 0},
    {XFA_Element::Pagination, 1, 0},
    {XFA_Element::Overprint, 1, 0},
    {XFA_Element::BehaviorOverride, 1, 0},
    {XFA_Element::Copies, 1, 0},
    {XFA_Element::Output, 1, 0},
    {XFA_Element::Validate, 1, 0},
    {XFA_Element::Layout, 1, 0},
    {XFA_Element::Script, 1, 0},
    {XFA_Element::Common, 1, 0},
    {XFA_Element::PaginationOverride, 1, 0},
    {XFA_Element::Destination, 1, 0},
    {XFA_Element::IncrementalMerge, 1, 0},
    {XFA_Element::Unknown, 0, 0}};

const XFA_Attribute attribute_data[] = {
    XFA_Attribute::Desc, XFA_Attribute::Lock, XFA_Attribute::Unknown};

const wchar_t* kName = L"present";

}  // namespace

CXFA_Present::CXFA_Present(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Config,
                XFA_ObjectType::Node,
                XFA_Element::Present,
                property_data,
                attribute_data,
                kName) {}

CXFA_Present::~CXFA_Present() {}
