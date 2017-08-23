// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_patterncs.h"

#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"

CPDF_PatternCS::CPDF_PatternCS(CPDF_Document* pDoc)
    : CPDF_ColorSpace(pDoc, PDFCS_PATTERN, 1),
      m_pBaseCS(nullptr),
      m_pCountedBaseCS(nullptr) {}

CPDF_PatternCS::~CPDF_PatternCS() {
  CPDF_ColorSpace* pCS = m_pCountedBaseCS ? m_pCountedBaseCS->get() : nullptr;
  if (pCS && m_pDocument) {
    auto* pPageData = m_pDocument->GetPageData();
    if (pPageData)
      pPageData->ReleaseColorSpace(pCS->GetArray());
  }
}

bool CPDF_PatternCS::v_Load(CPDF_Document* pDoc, CPDF_Array* pArray) {
  CPDF_Object* pBaseCS = pArray->GetDirectObjectAt(1);
  if (pBaseCS == m_pArray)
    return false;

  CPDF_DocPageData* pDocPageData = pDoc->GetPageData();
  m_pBaseCS = pDocPageData->GetColorSpace(pBaseCS, nullptr);
  if (!m_pBaseCS) {
    m_nComponents = 1;
    return true;
  }

  if (m_pBaseCS->GetFamily() == PDFCS_PATTERN)
    return false;

  m_pCountedBaseCS = pDocPageData->FindColorSpacePtr(m_pBaseCS->GetArray());
  m_nComponents = m_pBaseCS->CountComponents() + 1;
  return m_pBaseCS->CountComponents() <= MAX_PATTERN_COLORCOMPS;
}

pdfium::Optional<std::tuple<float, float, float>> CPDF_PatternCS::GetRGB(
    float* pBuf) const {
  if (!m_pBaseCS)
    return pdfium::Optional<std::tuple<float, float, float>>();

  ASSERT(m_pBaseCS->GetFamily() != PDFCS_PATTERN);
  PatternValue* pvalue = (PatternValue*)pBuf;
  return m_pBaseCS->GetRGB(pvalue->m_Comps);
}
