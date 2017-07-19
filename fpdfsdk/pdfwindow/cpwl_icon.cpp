// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pdfwindow/cpwl_icon.h"

#include <algorithm>
#include <sstream>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"

CPWL_Icon::CPWL_Icon() : m_pPDFStream(nullptr), m_pIconFit(nullptr) {}

CPWL_Icon::~CPWL_Icon() {}

std::pair<float, float> CPWL_Icon::GetImageSize() {
  if (!m_pPDFStream)
    return {0.0f, 0.0f};

  CPDF_Dictionary* pDict = m_pPDFStream->GetDict();
  if (!pDict)
    return {0.0f, 0.0f};

  CFX_FloatRect rect = pDict->GetRectFor("BBox");
  return {rect.right - rect.left, rect.top - rect.bottom};
}

CFX_Matrix CPWL_Icon::GetImageMatrix() {
  if (!m_pPDFStream)
    return CFX_Matrix();
  if (CPDF_Dictionary* pDict = m_pPDFStream->GetDict())
    return pDict->GetMatrixFor("Matrix");
  return CFX_Matrix();
}

CFX_ByteString CPWL_Icon::GetImageAlias() {
  if (!m_pPDFStream)
    return CFX_ByteString();
  if (CPDF_Dictionary* pDict = m_pPDFStream->GetDict())
    return pDict->GetStringFor("Name");
  return CFX_ByteString();
}

std::pair<float, float> CPWL_Icon::GetIconPosition() {
  if (!m_pIconFit)
    return {0.0f, 0.0f};

  CPDF_Array* pA =
      m_pIconFit->GetDict() ? m_pIconFit->GetDict()->GetArrayFor("A") : nullptr;
  if (!pA)
    return {0.0f, 0.0f};

  size_t dwCount = pA->GetCount();
  return {dwCount > 0 ? pA->GetNumberAt(0) : 0.0f,
          dwCount > 1 ? pA->GetNumberAt(1) : 0.0f};
}

std::pair<float, float> CPWL_Icon::GetScale() {
  float fHScale = 1.0f;
  float fVScale = 1.0f;

  if (!m_pPDFStream)
    return {fHScale, fVScale};

  CFX_FloatRect rcPlate = GetClientRect();
  float fPlateWidth = rcPlate.right - rcPlate.left;
  float fPlateHeight = rcPlate.top - rcPlate.bottom;

  float fImageWidth;
  float fImageHeight;
  std::tie(fImageWidth, fImageHeight) = GetImageSize();

  int32_t nScaleMethod = m_pIconFit ? m_pIconFit->GetScaleMethod() : 0;

  switch (nScaleMethod) {
    default:
    case 0:
      fHScale = fPlateWidth / std::max(fImageWidth, 1.0f);
      fVScale = fPlateHeight / std::max(fImageHeight, 1.0f);
      break;
    case 1:
      if (fPlateWidth < fImageWidth)
        fHScale = fPlateWidth / std::max(fImageWidth, 1.0f);
      if (fPlateHeight < fImageHeight)
        fVScale = fPlateHeight / std::max(fImageHeight, 1.0f);
      break;
    case 2:
      if (fPlateWidth > fImageWidth)
        fHScale = fPlateWidth / std::max(fImageWidth, 1.0f);
      if (fPlateHeight > fImageHeight)
        fVScale = fPlateHeight / std::max(fImageHeight, 1.0f);
      break;
    case 3:
      break;
  }

  float fMinScale;
  if (m_pIconFit && m_pIconFit->IsProportionalScale()) {
    fMinScale = std::min(fHScale, fVScale);
    fHScale = fMinScale;
    fVScale = fMinScale;
  }
  return {fHScale, fVScale};
}

std::pair<float, float> CPWL_Icon::GetImageOffset() {
  float fLeft;
  float fBottom;
  std::tie(fLeft, fBottom) = GetIconPosition();

  float fImageWidth;
  float fImageHeight;
  std::tie(fImageWidth, fImageHeight) = GetImageSize();

  float fHScale, fVScale;
  std::tie(fHScale, fVScale) = GetScale();

  float fImageFactWidth = fImageWidth * fHScale;
  float fImageFactHeight = fImageHeight * fVScale;

  CFX_FloatRect rcPlate = GetClientRect();
  float fPlateWidth = rcPlate.right - rcPlate.left;
  float fPlateHeight = rcPlate.top - rcPlate.bottom;

  return {(fPlateWidth - fImageFactWidth) * fLeft,
          (fPlateHeight - fImageFactHeight) * fBottom};
}
