// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_calculatedata.h"

#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_script.h"
#include "xfa/fxfa/parser/cxfa_text.h"

CXFA_CalculateData::CXFA_CalculateData(CXFA_Node* pNode)
    : CXFA_DataData(pNode) {}

XFA_AttributeEnum CXFA_CalculateData::GetOverride() const {
  return m_pNode->JSObject()
      ->TryEnum(XFA_Attribute::Override, false)
      .value_or(XFA_AttributeEnum::Error);
}

CXFA_Script* CXFA_CalculateData::GetScript() const {
  return static_cast<CXFA_Script*>(
      m_pNode->GetChild(0, XFA_Element::Script, false));
}

WideString CXFA_CalculateData::GetMessageText() const {
  CXFA_Node* pNode = m_pNode->GetChild(0, XFA_Element::Message, false);
  if (!pNode)
    return L"";

  CXFA_Text* text =
      static_cast<CXFA_Text*>(pNode->GetChild(0, XFA_Element::Text, false));
  return text ? text->GetContent() : L"";
}
