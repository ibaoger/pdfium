// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_form.h"

namespace {

const CXFA_Node::PropertyData property_data[] = {{XFA_Element::Unknown, 0, 0}};

const XFA_Attribute attribute_data[] = {XFA_Attribute::Checksum,
                                        XFA_Attribute::Unknown};

const wchar_t* kName = L"form";

}  // namespace

CXFA_Form::CXFA_Form(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Form,
                XFA_ObjectType::ModelNode,
                XFA_Element::Form,
                property_data,
                attribute_data,
                kName) {}

CXFA_Form::~CXFA_Form() {}
