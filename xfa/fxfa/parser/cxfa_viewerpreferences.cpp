// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_viewerpreferences.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::PrintScaling, 1, 0},
    {XFA_Element::Enforce, 1, 0},
    {XFA_Element::NumberOfCopies, 1, 0},
    {XFA_Element::PageRange, 1, 0},
    {XFA_Element::AddViewerPreferences, 1, 0},
    {XFA_Element::ADBE_JSConsole, 1, 0},
    {XFA_Element::DuplexOption, 1, 0},
    {XFA_Element::ADBE_JSDebugger, 1, 0},
    {XFA_Element::PickTrayByPDFSize, 1, 0},
    {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Desc, XFA_Attribute::Lock, XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"viewerPreferences";

}  // namespace

CXFA_ViewerPreferences::CXFA_ViewerPreferences(CXFA_Document* doc,
                                               XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Config,
                XFA_ObjectType::Node,
                XFA_Element::ViewerPreferences,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_ViewerPreferences::~CXFA_ViewerPreferences() {}
