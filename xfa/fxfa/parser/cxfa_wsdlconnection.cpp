// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_wsdlconnection.h"

namespace {

const CXFA_Node::PropertyData property_data[] = {
    {XFA_Element::Operation, 1, 0},
    {XFA_Element::WsdlAddress, 1, 0},
    {XFA_Element::SoapAddress, 1, 0},
    {XFA_Element::SoapAction, 1, 0},
    {XFA_Element::EffectiveOutputPolicy, 1, 0},
    {XFA_Element::EffectiveInputPolicy, 1, 0},
    {XFA_Element::Unknown, 0, 0}};

const XFA_Attribute attribute_data[] = {XFA_Attribute::Name,
                                        XFA_Attribute::DataDescription,
                                        XFA_Attribute::Unknown};

const wchar_t* kName = L"wsdlConnection";

}  // namespace

CXFA_WsdlConnection::CXFA_WsdlConnection(CXFA_Document* doc,
                                         XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_ConnectionSet,
                XFA_ObjectType::Node,
                XFA_Element::WsdlConnection,
                property_data,
                attribute_data,
                kName) {}

CXFA_WsdlConnection::~CXFA_WsdlConnection() {}
