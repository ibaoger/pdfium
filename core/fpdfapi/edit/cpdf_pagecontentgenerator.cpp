// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/edit/cpdf_pagecontentgenerator.h"

#include <tuple>
#include <utility>

#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_path.h"
#include "core/fpdfapi/page/cpdf_pathobject.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"

namespace {

CFX_ByteTextBuf& operator<<(CFX_ByteTextBuf& ar, const CFX_Matrix& matrix) {
  ar << matrix.a << " " << matrix.b << " " << matrix.c << " " << matrix.d << " "
     << matrix.e << " " << matrix.f;
  return ar;
}

bool GetColor(const CPDF_Color* pColor, FX_FLOAT* rgb) {
  int intRGB[3];
  if (!pColor ||
      pColor->GetColorSpace() != CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB) ||
      !pColor->GetRGB(intRGB[0], intRGB[1], intRGB[2])) {
    return false;
  }
  rgb[0] = intRGB[0] / 255.0f;
  rgb[1] = intRGB[1] / 255.0f;
  rgb[2] = intRGB[2] / 255.0f;
  return true;
}

}  // namespace

CPDF_PageContentGenerator::CPDF_PageContentGenerator(CPDF_Page* pPage)
    : m_pPage(pPage), m_pDocument(m_pPage->m_pDocument) {
  for (const auto& pObj : *pPage->GetPageObjectList()) {
    if (pObj)
      m_pageObjects.push_back(pObj.get());
  }
}

CPDF_PageContentGenerator::~CPDF_PageContentGenerator() {}

void CPDF_PageContentGenerator::GenerateContent() {
  CFX_ByteTextBuf buf;
  for (CPDF_PageObject* pPageObj : m_pageObjects) {
    if (CPDF_ImageObject* pImageObject = pPageObj->AsImage())
      ProcessImage(&buf, pImageObject);
    else if (CPDF_PathObject* pPathObj = pPageObj->AsPath())
      ProcessPath(&buf, pPathObj);
  }
  CPDF_Dictionary* pPageDict = m_pPage->m_pFormDict;
  CPDF_Object* pContent =
      pPageDict ? pPageDict->GetDirectObjectFor("Contents") : nullptr;
  if (pContent)
    pPageDict->RemoveFor("Contents");

  CPDF_Stream* pStream = m_pDocument->NewIndirect<CPDF_Stream>();
  pStream->SetData(buf.GetBuffer(), buf.GetLength());
  pPageDict->SetNewFor<CPDF_Reference>("Contents", m_pDocument,
                                       pStream->GetObjNum());
}

CFX_ByteString CPDF_PageContentGenerator::RealizeResource(
    uint32_t dwResourceObjNum,
    const CFX_ByteString& bsType) {
  ASSERT(dwResourceObjNum);
  if (!m_pPage->m_pResources) {
    m_pPage->m_pResources = m_pDocument->NewIndirect<CPDF_Dictionary>();
    m_pPage->m_pFormDict->SetNewFor<CPDF_Reference>(
        "Resources", m_pDocument, m_pPage->m_pResources->GetObjNum());
  }
  CPDF_Dictionary* pResList = m_pPage->m_pResources->GetDictFor(bsType);
  if (!pResList)
    pResList = m_pPage->m_pResources->SetNewFor<CPDF_Dictionary>(bsType);

  CFX_ByteString name;
  int idnum = 1;
  while (1) {
    name.Format("FX%c%d", bsType[0], idnum);
    if (!pResList->KeyExist(name)) {
      break;
    }
    idnum++;
  }
  pResList->SetNewFor<CPDF_Reference>(name, m_pDocument, dwResourceObjNum);
  return name;
}

void CPDF_PageContentGenerator::ProcessImage(CFX_ByteTextBuf* buf,
                                             CPDF_ImageObject* pImageObj) {
  if ((pImageObj->matrix().a == 0 && pImageObj->matrix().b == 0) ||
      (pImageObj->matrix().c == 0 && pImageObj->matrix().d == 0)) {
    return;
  }
  *buf << "q " << pImageObj->matrix() << " cm ";

  CPDF_Image* pImage = pImageObj->GetImage();
  if (pImage->IsInline())
    return;

  CPDF_Stream* pStream = pImage->GetStream();
  if (!pStream)
    return;

  bool bWasInline = pStream->IsInline();
  if (bWasInline)
    pImage->ConvertStreamToIndirectObject();

  uint32_t dwObjNum = pStream->GetObjNum();
  CFX_ByteString name = RealizeResource(dwObjNum, "XObject");
  if (bWasInline)
    pImageObj->SetUnownedImage(m_pDocument->GetPageData()->GetImage(dwObjNum));

  *buf << "/" << PDF_NameEncode(name) << " Do Q\n";
}

// Processing path with operators from Tables 4.9 and 4.10 of PDF spec 1.7:
// "re" appends a rectangle (here, used only if the whole path is a rectangle)
// "m" moves current point to the given coordinates
// "l" creates a line from current point to the new point
// "c" adds a Bezier curve from current to last point, using the two other
// points as the Bezier control points
// Note: "l", "c" change the current point
// "h" closes the subpath (appends a line from current to starting point)
// Path painting operators: "S", "n", "B", "f", "B*", "f*", depending on
// the filling mode and whether we want stroking the path or not.
// "Q" restores the graphics state imposed by the ProcessGraphics method.
void CPDF_PageContentGenerator::ProcessPath(CFX_ByteTextBuf* buf,
                                            CPDF_PathObject* pPathObj) {
  ProcessGraphics(buf, pPathObj);
  const FX_PATHPOINT* pPoints = pPathObj->m_Path.GetPoints();
  if (pPathObj->m_Path.IsRect()) {
    *buf << pPoints[0].m_PointX << " " << pPoints[0].m_PointY << " "
         << (pPoints[2].m_PointX - pPoints[0].m_PointX) << " "
         << (pPoints[2].m_PointY - pPoints[0].m_PointY) << " re";
  } else {
    int numPoints = pPathObj->m_Path.GetPointCount();
    for (int i = 0; i < numPoints; i++) {
      if (i > 0)
        *buf << " ";
      *buf << pPoints[i].m_PointX << " " << pPoints[i].m_PointY;
      FXPT_TYPE pointType = pPoints[i].m_Type;
      if (pointType == FXPT_TYPE::MoveTo) {
        *buf << " m";
      } else if (pointType == FXPT_TYPE::LineTo) {
        *buf << " l";
      } else if (pointType == FXPT_TYPE::BezierTo) {
        if (i + 2 >= numPoints ||
            !pPoints[i].IsTypeAndOpen(FXPT_TYPE::BezierTo) ||
            !pPoints[i + 1].IsTypeAndOpen(FXPT_TYPE::BezierTo) ||
            pPoints[i + 2].m_Type != FXPT_TYPE::BezierTo) {
          // If format is not supported, close the path and paint
          *buf << " h";
          break;
        }
        *buf << " " << pPoints[i + 1].m_PointX << " " << pPoints[i + 1].m_PointY
             << " " << pPoints[i + 2].m_PointX << " " << pPoints[i + 2].m_PointY
             << " c";
        i += 2;
      }
      if (pPoints[i].m_CloseFigure)
        *buf << " h";
    }
  }
  if (pPathObj->m_FillType == 0)
    *buf << (pPathObj->m_bStroke ? " S" : " n");
  else if (pPathObj->m_FillType == FXFILL_WINDING)
    *buf << (pPathObj->m_bStroke ? " B" : " f");
  else if (pPathObj->m_FillType == FXFILL_ALTERNATE)
    *buf << (pPathObj->m_bStroke ? " B*" : " f*");
  *buf << " Q\n";
}

// This method supports color operators rg and RGB from Table 4.24 of PDF spec
// 1.7. A color will not be set if the colorspace is not DefaultRGB or the RGB
// values cannot be obtained. The method also adds an external graphics
// dictionary, as described in Section 4.3.4.
// "rg" sets the fill color, "RG" sets the stroke color (using DefaultRGB)
// "ca" sets the fill alpha, "CA" sets the stroke alpha.
// "q" saves the graphics state, so that the settings can later be reversed
void CPDF_PageContentGenerator::ProcessGraphics(CFX_ByteTextBuf* buf,
                                                CPDF_PageObject* pPageObj) {
  *buf << "q ";
  FX_FLOAT fillColor[3];
  if (GetColor(pPageObj->m_ColorState.GetFillColor(), fillColor)) {
    *buf << fillColor[0] << " " << fillColor[1] << " " << fillColor[2]
         << " rg ";
  }
  FX_FLOAT strokeColor[3];
  if (GetColor(pPageObj->m_ColorState.GetStrokeColor(), strokeColor)) {
    *buf << strokeColor[0] << " " << strokeColor[1] << " " << strokeColor[2]
         << " RG ";
  }

  GraphicsData graphD;
  graphD.fillAlpha = pPageObj->m_GeneralState.GetFillAlpha();
  graphD.strokeAlpha = pPageObj->m_GeneralState.GetStrokeAlpha();
  if (graphD.fillAlpha == 1.0f && graphD.strokeAlpha == 1.0f)
    return;

  CFX_ByteString name;
  auto it = m_GraphicsMap.find(graphD);
  if (it != m_GraphicsMap.end()) {
    name = it->second;
  } else {
    auto gsDict = pdfium::MakeUnique<CPDF_Dictionary>();
    gsDict->SetNewFor<CPDF_Number>("ca", graphD.fillAlpha);
    gsDict->SetNewFor<CPDF_Number>("CA", graphD.strokeAlpha);
    CPDF_Object* pDict = m_pDocument->AddIndirectObject(std::move(gsDict));
    uint32_t dwObjNum = pDict->GetObjNum();
    name = RealizeResource(dwObjNum, "ExtGState");
    m_GraphicsMap[graphD] = name;
  }
  *buf << "/" << PDF_NameEncode(name) << " gs ";
}

bool CPDF_PageContentGenerator::GraphicsData::operator<(
    const GraphicsData& other) const {
  if (fillAlpha != other.fillAlpha)
    return fillAlpha < other.fillAlpha;
  return strokeAlpha < other.strokeAlpha;
}
