// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_margindata.h"

#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_MarginData::CXFA_MarginData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

pdfium::Optional<float> CXFA_MarginData::TryInset(XFA_Attribute inset) const {
  pdfium::Optional<CXFA_Measurement> measure =
      GetNode()->JSNode()->TryMeasure(inset, false);
  if (measure)
    return {measure->ToUnit(XFA_Unit::Pt)};
  return {};
}

pdfium::Optional<float> CXFA_MarginData::TryLeftInset() const {
  return TryInset(XFA_Attribute::LeftInset);
}

pdfium::Optional<float> CXFA_MarginData::TryTopInset() const {
  return TryInset(XFA_Attribute::TopInset);
}

pdfium::Optional<float> CXFA_MarginData::TryRightInset() const {
  return TryInset(XFA_Attribute::RightInset);
}

pdfium::Optional<float> CXFA_MarginData::TryBottomInset() const {
  return TryInset(XFA_Attribute::BottomInset);
}

float CXFA_MarginData::GetLeftInset() const {
  return TryLeftInset().value_or(0.0f);
}

float CXFA_MarginData::GetRightInset() const {
  return TryRightInset().value_or(0.0f);
}

float CXFA_MarginData::GetTopInset() const {
  return TryTopInset().value_or(0.0f);
}

float CXFA_MarginData::GetBottomInset() const {
  return TryBottomInset().value_or(0.0f);
}
