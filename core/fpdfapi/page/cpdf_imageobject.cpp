// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_imageobject.h"

#include <memory>

#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/parser/cpdf_document.h"

CPDF_ImageObject::CPDF_ImageObject() {}

CPDF_ImageObject::~CPDF_ImageObject() {
  if (m_pImage) {
    CPDF_DocPageData* pPageData = m_pImage->GetDocument()->GetPageData();
    pPageData->MaybePurgeImage(m_pImage->GetStream()->GetObjNum());
  }
}

CPDF_PageObject::Type CPDF_ImageObject::GetType() const {
  return IMAGE;
}

void CPDF_ImageObject::Transform(const CFX_Matrix& matrix) {
  m_Matrix.Concat(matrix);
  CalcBoundingBox();
}

bool CPDF_ImageObject::IsImage() const {
  return true;
}

CPDF_ImageObject* CPDF_ImageObject::AsImage() {
  return this;
}

const CPDF_ImageObject* CPDF_ImageObject::AsImage() const {
  return this;
}

void CPDF_ImageObject::CalcBoundingBox() {
  m_Left = 0;
  m_Bottom = 0;
  m_Right = 1.0f;
  m_Top = 1.0f;
  m_Matrix.TransformRect(m_Left, m_Right, m_Top, m_Bottom);
}
