// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_common.h"

namespace {

const CXFA_Node::PropertyData property_data[] = {
    {XFA_Element::SuppressBanner, 1, 0},
    {XFA_Element::VersionControl, 1, 0},
    {XFA_Element::LocaleSet, 1, 0},
    {XFA_Element::Template, 1, 0},
    {XFA_Element::ValidationMessaging, 1, 0},
    {XFA_Element::Locale, 1, 0},
    {XFA_Element::Data, 1, 0},
    {XFA_Element::Messaging, 1, 0},
    {XFA_Element::Unknown, 0, 0}};

const XFA_Attribute attribute_data[] = {
    XFA_Attribute::Desc, XFA_Attribute::Lock, XFA_Attribute::Unknown};

const wchar_t* kName = L"common";

}  // namespace

CXFA_Common::CXFA_Common(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Config,
                XFA_ObjectType::Node,
                XFA_Element::Common,
                property_data,
                attribute_data,
                kName) {}

CXFA_Common::~CXFA_Common() {}
