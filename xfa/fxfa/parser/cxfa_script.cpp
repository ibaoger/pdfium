// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_script.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::Exclude, 1, 0},
    {XFA_Element::CurrentPage, 1, 0},
    {XFA_Element::RunScripts, 1, 0},
    {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Id,          XFA_Attribute::Name,  XFA_Attribute::Use,
    XFA_Attribute::ContentType, XFA_Attribute::RunAt, XFA_Attribute::Binding,
    XFA_Attribute::Usehref,     XFA_Attribute::Desc,  XFA_Attribute::Lock,
    XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"script";

}  // namespace

CXFA_Script::CXFA_Script(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(
          doc,
          packet,
          (XFA_XDPPACKET_Config | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
          XFA_ObjectType::ContentNode,
          XFA_Element::Script,
          kPropertyData,
          kAttributeData,
          kName) {}

CXFA_Script::~CXFA_Script() {}
