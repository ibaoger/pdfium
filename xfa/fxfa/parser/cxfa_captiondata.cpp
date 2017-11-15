// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_captiondata.h"

#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_CaptionData::CXFA_CaptionData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

int32_t CXFA_CaptionData::GetPresence() {
  pdfium::Optional<XFA_ATTRIBUTEENUM> attr =
      m_pNode->JSNode()->TryEnum(XFA_Attribute::Presence, true);
  return attr ? *attr : XFA_ATTRIBUTEENUM_Visible;
}

int32_t CXFA_CaptionData::GetPlacementType() {
  pdfium::Optional<XFA_ATTRIBUTEENUM> attr =
      m_pNode->JSNode()->TryEnum(XFA_Attribute::Placement, true);
  return attr ? *attr : XFA_ATTRIBUTEENUM_Left;
}

float CXFA_CaptionData::GetReserve() {
  CXFA_Measurement ms;
  m_pNode->JSNode()->TryMeasure(XFA_Attribute::Reserve, ms, true);
  return ms.ToUnit(XFA_Unit::Pt);
}

CXFA_MarginData CXFA_CaptionData::GetMarginData() {
  return CXFA_MarginData(
      m_pNode ? m_pNode->GetChild(0, XFA_Element::Margin, false) : nullptr);
}

CXFA_FontData CXFA_CaptionData::GetFontData() {
  return CXFA_FontData(m_pNode ? m_pNode->GetChild(0, XFA_Element::Font, false)
                               : nullptr);
}

CXFA_ValueData CXFA_CaptionData::GetValueData() {
  return CXFA_ValueData(
      m_pNode ? m_pNode->GetChild(0, XFA_Element::Value, false) : nullptr);
}
