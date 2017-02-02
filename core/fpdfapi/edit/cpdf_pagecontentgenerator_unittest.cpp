// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/edit/cpdf_pagecontentgenerator.h"

#include "core/fpdfapi/page/cpdf_pathobject.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

TEST(cpdf_pagecontentgenerator, ProcessRect) {
  auto pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
  pPathObj->m_Path.AppendRect(10, 5, 13, 30);
  pPathObj->m_FillType = FXFILL_ALTERNATE;
  pPathObj->m_bStroke = true;
  CFX_ByteTextBuf buf;
  CPDF_PageContentGenerator::ProcessPath(&buf, pPathObj.get());
  const CFX_ByteString rectangle1 = "10 5 3 25 re B*\n";
  EXPECT_EQ(rectangle1, buf.MakeString());

  pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
  pPathObj->m_Path.SetPointCount(4);
  FX_PATHPOINT* pPoints = pPathObj->m_Path.GetMutablePoints();
  pPoints[0].m_PointX = 0;
  pPoints[0].m_PointY = 0;
  pPoints[0].m_Flag = FXPT_MOVETO;
  pPoints[1].m_PointX = static_cast<FX_FLOAT>(5.2);
  pPoints[1].m_PointY = 0;
  pPoints[1].m_Flag = FXPT_LINETO;
  pPoints[2].m_PointX = static_cast<FX_FLOAT>(5.2);
  pPoints[2].m_PointY = static_cast<FX_FLOAT>(3.78);
  pPoints[2].m_Flag = FXPT_LINETO;
  pPoints[3].m_PointX = 0;
  pPoints[3].m_PointY = static_cast<FX_FLOAT>(3.78);
  pPoints[3].m_Flag = FXPT_LINETO | FXPT_CLOSEFIGURE;
  buf.Clear();
  CPDF_PageContentGenerator::ProcessPath(&buf, pPathObj.get());
  const CFX_ByteString rectangle2 = "0 0 5.2 3.78 re n\n";
  EXPECT_EQ(rectangle2, buf.MakeString());
}

TEST(cpdf_pagecontentgenerator, ProcessPath) {
  auto pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
  pPathObj->m_Path.SetPointCount(10);
  FX_PATHPOINT* pPoints = pPathObj->m_Path.GetMutablePoints();
  pPoints[0].m_PointX = static_cast<FX_FLOAT>(3.102);
  pPoints[0].m_PointY = static_cast<FX_FLOAT>(4.67);
  pPoints[0].m_Flag = FXPT_MOVETO;
  pPoints[1].m_PointX = static_cast<FX_FLOAT>(5.45);
  pPoints[1].m_PointY = static_cast<FX_FLOAT>(0.29);
  pPoints[1].m_Flag = FXPT_LINETO;
  pPoints[2].m_PointX = static_cast<FX_FLOAT>(4.24);
  pPoints[2].m_PointY = static_cast<FX_FLOAT>(3.15);
  pPoints[2].m_Flag = FXPT_BEZIERTO;
  pPoints[3].m_PointX = static_cast<FX_FLOAT>(4.65);
  pPoints[3].m_PointY = static_cast<FX_FLOAT>(2.98);
  pPoints[3].m_Flag = FXPT_BEZIERTO;
  pPoints[4].m_PointX = static_cast<FX_FLOAT>(3.456);
  pPoints[4].m_PointY = static_cast<FX_FLOAT>(0.24);
  pPoints[4].m_Flag = FXPT_BEZIERTO;
  pPoints[5].m_PointX = static_cast<FX_FLOAT>(10.6);
  pPoints[5].m_PointY = static_cast<FX_FLOAT>(11.15);
  pPoints[5].m_Flag = FXPT_LINETO;
  pPoints[6].m_PointX = 11;
  pPoints[6].m_PointY = static_cast<FX_FLOAT>(12.5);
  pPoints[6].m_Flag = FXPT_LINETO;
  pPoints[7].m_PointX = static_cast<FX_FLOAT>(11.46);
  pPoints[7].m_PointY = static_cast<FX_FLOAT>(12.67);
  pPoints[7].m_Flag = FXPT_BEZIERTO;
  pPoints[8].m_PointX = static_cast<FX_FLOAT>(11.84);
  pPoints[8].m_PointY = static_cast<FX_FLOAT>(12.96);
  pPoints[8].m_Flag = FXPT_BEZIERTO;
  pPoints[9].m_PointX = 12;
  pPoints[9].m_PointY = static_cast<FX_FLOAT>(13.64);
  pPoints[9].m_Flag = FXPT_BEZIERTO | FXPT_CLOSEFIGURE;
  pPathObj->m_FillType = FXFILL_WINDING;
  pPathObj->m_bStroke = false;
  CFX_ByteTextBuf buf;
  CPDF_PageContentGenerator::ProcessPath(&buf, pPathObj.get());
  const CFX_ByteString rectangle =
      "3.102 4.67 m 5.45 0.29 l 4.24 3.15 4.65 2.98 3.456 0.24 c 10.6 11.15 l "
      "11 12.5 l 11.46 12.67 11.84 12.96 12 13.64 c h f\n";
  EXPECT_EQ(rectangle, buf.MakeString());
}
