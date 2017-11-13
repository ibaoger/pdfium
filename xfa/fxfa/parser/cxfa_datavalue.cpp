// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_datavalue.h"

namespace {

const XFA_Attribute attribute_data[] = {
    XFA_Attribute::Name,  XFA_Attribute::ContentType, XFA_Attribute::Contains,
    XFA_Attribute::Value, XFA_Attribute::IsNull,      XFA_Attribute::Unknown};

const wchar_t* kName = L"dataValue";

}  // namespace

CXFA_DataValue::CXFA_DataValue(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Datasets,
                XFA_ObjectType::Node,
                XFA_Element::DataValue,
                nullptr,
                attribute_data,
                kName) {}

CXFA_DataValue::~CXFA_DataValue() {}
