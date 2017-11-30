// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_calendarsymbols.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::EraNames, 1, 0},
    {XFA_Element::DayNames, 2, 0},
    {XFA_Element::MeridiemNames, 1, 0},
    {XFA_Element::MonthNames, 2, 0},
    {XFA_Element::Unknown, 0, 0}};
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Name, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Gregorian},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, nullptr}};

constexpr wchar_t kName[] = L"calendarSymbols";

}  // namespace

CXFA_CalendarSymbols::CXFA_CalendarSymbols(CXFA_Document* doc,
                                           XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_LocaleSet,
                XFA_ObjectType::Node,
                XFA_Element::CalendarSymbols,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_CalendarSymbols::~CXFA_CalendarSymbols() {}
