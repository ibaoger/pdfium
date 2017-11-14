// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_datagroup.h"

namespace {

const XFA_Attribute attribute_data[] = {XFA_Attribute::Name,
                                        XFA_Attribute::Unknown};

const wchar_t* kName = L"dataGroup";

}  // namespace

CXFA_DataGroup::CXFA_DataGroup(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Datasets,
                XFA_ObjectType::Node,
                XFA_Element::DataGroup,
                nullptr,
                attribute_data,
                kName) {}

CXFA_DataGroup::~CXFA_DataGroup() {}
