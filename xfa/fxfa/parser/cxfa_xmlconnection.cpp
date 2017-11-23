// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_xmlconnection.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {{XFA_Element::Uri, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute kAttributeData[] = {XFA_Attribute::Name,
                                        XFA_Attribute::DataDescription,
                                        XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"xmlConnection";

}  // namespace

CXFA_XmlConnection::CXFA_XmlConnection(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_ConnectionSet,
                XFA_ObjectType::Node,
                XFA_Element::XmlConnection,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_XmlConnection::~CXFA_XmlConnection() {}
