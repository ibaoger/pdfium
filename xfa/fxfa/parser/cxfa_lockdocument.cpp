// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_lockdocument.h"

namespace {

const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Type, XFA_AttributeType::Enum,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form,
     (void*)XFA_ATTRIBUTEENUM_Optional},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, 0, nullptr}};

constexpr wchar_t kName[] = L"lockDocument";

}  // namespace

CXFA_LockDocument::CXFA_LockDocument(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::ContentNode,
                XFA_Element::LockDocument,
                nullptr,
                kAttributeData,
                kName) {}

CXFA_LockDocument::~CXFA_LockDocument() {}
