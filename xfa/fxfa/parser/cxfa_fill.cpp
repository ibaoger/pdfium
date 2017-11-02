// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_fill.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_Fill::CXFA_Fill(CXFA_Node* pNode) : CXFA_Data(pNode) {}

CXFA_Fill::~CXFA_Fill() {}

int32_t CXFA_Fill::GetPresence() {
  return m_pNode->JSNode()->GetEnum(XFA_ATTRIBUTE_Presence);
}

void CXFA_Fill::SetColor(FX_ARGB color) {
  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Color, true);
  WideString wsColor;
  int a;
  int r;
  int g;
  int b;
  std::tie(a, r, g, b) = ArgbDecode(color);
  wsColor.Format(L"%d,%d,%d", r, g, b);
  pNode->JSNode()->SetCData(XFA_ATTRIBUTE_Value, wsColor, false, false);
}

FX_ARGB CXFA_Fill::GetColor(bool bText) {
  if (CXFA_Node* pNode = m_pNode->GetChild(0, XFA_Element::Color)) {
    WideStringView wsColor;
    if (pNode->JSNode()->TryCData(XFA_ATTRIBUTE_Value, wsColor, false))
      return CXFA_Data::ToColor(wsColor);
  }
  if (bText)
    return 0xFF000000;
  return 0xFFFFFFFF;
}

XFA_Element CXFA_Fill::GetFillType() {
  CXFA_Node* pChild = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pChild) {
    XFA_Element eType = pChild->GetElementType();
    if (eType != XFA_Element::Color && eType != XFA_Element::Extras)
      return eType;

    pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return XFA_Element::Solid;
}

int32_t CXFA_Fill::GetPattern(FX_ARGB& foreColor) {
  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Pattern, true);
  if (CXFA_Node* pColor = pNode->GetChild(0, XFA_Element::Color)) {
    WideStringView wsColor;
    pColor->JSNode()->TryCData(XFA_ATTRIBUTE_Value, wsColor, false);
    foreColor = CXFA_Data::ToColor(wsColor);
  } else {
    foreColor = 0xFF000000;
  }
  return pNode->JSNode()->GetEnum(XFA_ATTRIBUTE_Type);
}

int32_t CXFA_Fill::GetStipple(FX_ARGB& stippleColor) {
  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Stipple, true);
  int32_t eAttr = 50;
  pNode->JSNode()->TryInteger(XFA_ATTRIBUTE_Rate, eAttr, true);
  if (CXFA_Node* pColor = pNode->GetChild(0, XFA_Element::Color)) {
    WideStringView wsColor;
    pColor->JSNode()->TryCData(XFA_ATTRIBUTE_Value, wsColor, false);
    stippleColor = CXFA_Data::ToColor(wsColor);
  } else {
    stippleColor = 0xFF000000;
  }
  return eAttr;
}

int32_t CXFA_Fill::GetLinear(FX_ARGB& endColor) {
  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Linear, true);
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_ToRight;
  pNode->JSNode()->TryEnum(XFA_ATTRIBUTE_Type, eAttr);
  if (CXFA_Node* pColor = pNode->GetChild(0, XFA_Element::Color)) {
    WideStringView wsColor;
    pColor->JSNode()->TryCData(XFA_ATTRIBUTE_Value, wsColor, false);
    endColor = CXFA_Data::ToColor(wsColor);
  } else {
    endColor = 0xFF000000;
  }
  return eAttr;
}

int32_t CXFA_Fill::GetRadial(FX_ARGB& endColor) {
  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Radial, true);
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_ToEdge;
  pNode->JSNode()->TryEnum(XFA_ATTRIBUTE_Type, eAttr);
  if (CXFA_Node* pColor = pNode->GetChild(0, XFA_Element::Color)) {
    WideStringView wsColor;
    pColor->JSNode()->TryCData(XFA_ATTRIBUTE_Value, wsColor, false);
    endColor = CXFA_Data::ToColor(wsColor);
  } else {
    endColor = 0xFF000000;
  }
  return eAttr;
}
