// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_pagemodule.h"
#include "third_party/base/ptr_util.h"

CPDF_PageModule::CPDF_PageModule()
    : m_StockGrayCS(
          pdfium::MakeRetain<CPDF_DeviceCS>(nullptr, PDFCS_DEVICEGRAY)),
      m_StockRGBCS(pdfium::MakeRetain<CPDF_DeviceCS>(nullptr, PDFCS_DEVICERGB)),
      m_StockCMYKCS(
          pdfium::MakeRetain<CPDF_DeviceCS>(nullptr, PDFCS_DEVICECMYK)),
      m_StockPatternCS(pdfium::MakeRetain<CPDF_PatternCS>(nullptr)) {}

CPDF_PageModule::~CPDF_PageModule() {}

CPDF_FontGlobals* CPDF_PageModule::GetFontGlobals() {
  return &m_FontGlobals;
}

CFX_RetainPtr<CPDF_ColorSpace> CPDF_PageModule::GetStockCS(int family) {
  if (family == PDFCS_DEVICEGRAY)
    return m_StockGrayCS;
  if (family == PDFCS_DEVICERGB)
    return m_StockRGBCS;
  if (family == PDFCS_DEVICECMYK)
    return m_StockCMYKCS;
  if (family == PDFCS_PATTERN)
    return m_StockPatternCS;
  return nullptr;
}

void CPDF_PageModule::ClearStockFont(CPDF_Document* pDoc) {
  m_FontGlobals.Clear(pDoc);
}
