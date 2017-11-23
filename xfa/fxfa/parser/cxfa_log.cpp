// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_log.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {{XFA_Element::To, 1, 0},
                                                 {XFA_Element::Uri, 1, 0},
                                                 {XFA_Element::Mode, 1, 0},
                                                 {XFA_Element::Threshold, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Desc, XFA_Attribute::Lock, XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"log";

}  // namespace

CXFA_Log::CXFA_Log(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Config,
                XFA_ObjectType::Node,
                XFA_Element::Log,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Log::~CXFA_Log() {}
