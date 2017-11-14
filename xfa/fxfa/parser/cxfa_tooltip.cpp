// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_tooltip.h"

namespace {

const XFA_Attribute attribute_data[] = {
    XFA_Attribute::Id, XFA_Attribute::Rid, XFA_Attribute::Use,
    XFA_Attribute::Usehref, XFA_Attribute::Unknown};

const wchar_t* kName = L"toolTip";

}  // namespace

CXFA_ToolTip::CXFA_ToolTip(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::TextNode,
                XFA_Element::ToolTip,
                nullptr,
                attribute_data,
                kName) {}

CXFA_ToolTip::~CXFA_ToolTip() {}
