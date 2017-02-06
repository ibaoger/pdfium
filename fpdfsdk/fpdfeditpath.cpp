// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_edit.h"

#include "core/fpdfapi/page/cpdf_path.h"
#include "core/fpdfapi/page/cpdf_pathobject.h"
#include "core/fxcrt/fx_system.h"

DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFPageObj_CreateNewPath(float x, float y) {
  CPDF_PathObject* pPathObj = new CPDF_PathObject;
  pPathObj->m_Path.AppendPoint(x, y, FXPT_MOVETO);
  return pPathObj;
}

DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFPageObj_CreateNewRect(float x,
                                                            float y,
                                                            float w,
                                                            float h) {
  CPDF_PathObject* pPathObj = new CPDF_PathObject;
  pPathObj->m_Path.AppendRect(x, y, x + w, y + h);
  return pPathObj;
}

DLLEXPORT FPDF_BOOL FPDFPath_SetStrokeColor(FPDF_PAGEOBJECT path,
                                            unsigned int R,
                                            unsigned int G,
                                            unsigned int B,
                                            unsigned int A) {
  if (!path || R > 255 || G > 255 || B > 255 || A > 255)
    return false;

  CPDF_PathObject* pPathObj = reinterpret_cast<CPDF_PathObject*>(path);
  pPathObj->m_GeneralState.SetStrokeAlpha(A / 255.f);
  FX_FLOAT rgb[3] = {R / 255.f, G / 255.f, B / 255.f};
  pPathObj->m_ColorState.SetStrokeColor(
      CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB), rgb, 3);
  return true;
}

DLLEXPORT FPDF_BOOL FPDFPath_SetFillColor(FPDF_PAGEOBJECT path,
                                          unsigned int R,
                                          unsigned int G,
                                          unsigned int B,
                                          unsigned int A) {
  if (!path || R > 255 || G > 255 || B > 255 || A > 255)
    return false;

  CPDF_PathObject* pPathObj = reinterpret_cast<CPDF_PathObject*>(path);
  pPathObj->m_GeneralState.SetFillAlpha(A / 255.f);
  FX_FLOAT rgb[3] = {R / 255.f, G / 255.f, B / 255.f};
  pPathObj->m_ColorState.SetFillColor(
      CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB), rgb, 3);
  return true;
}

DLLEXPORT FPDF_BOOL FPDFPath_MoveTo(FPDF_PAGEOBJECT path, float x, float y) {
  if (!path)
    return false;

  CPDF_PathObject* pPathObj = reinterpret_cast<CPDF_PathObject*>(path);
  pPathObj->m_Path.AppendPoint(x, y, FXPT_MOVETO);
  return true;
}

DLLEXPORT FPDF_BOOL FPDFPath_LineTo(FPDF_PAGEOBJECT path, float x, float y) {
  if (!path)
    return false;

  CPDF_PathObject* pPathObj = reinterpret_cast<CPDF_PathObject*>(path);
  pPathObj->m_Path.AppendPoint(x, y, FXPT_LINETO);
  return true;
}

DLLEXPORT FPDF_BOOL FPDF_PathBezierTo(FPDF_PAGEOBJECT path,
                                      float x1,
                                      float y1,
                                      float x2,
                                      float y2,
                                      float x3,
                                      float y3) {
  if (!path)
    return false;

  CPDF_PathObject* pPathObj = reinterpret_cast<CPDF_PathObject*>(path);
  pPathObj->m_Path.AppendPoint(x1, y1, FXPT_BEZIERTO);
  pPathObj->m_Path.AppendPoint(x2, y2, FXPT_BEZIERTO);
  pPathObj->m_Path.AppendPoint(x3, y3, FXPT_BEZIERTO);
  return true;
}

DLLEXPORT FPDF_BOOL FPDF_PathClose(FPDF_PAGEOBJECT path) {
  if (!path)
    return false;

  CPDF_PathObject* pPathObj = reinterpret_cast<CPDF_PathObject*>(path);
  int numPoints = pPathObj->m_Path.GetPointCount();
  if (numPoints == 0)
    return false;

  FX_PATHPOINT* pPoints = pPathObj->m_Path.GetMutablePoints();
  if (pPoints[numPoints - 1].m_Flag & FXPT_CLOSEFIGURE)
    return true;

  pPoints[numPoints - 1].m_Flag |= FXPT_CLOSEFIGURE;
  return true;
}

DLLEXPORT FPDF_BOOL FPDF_PathSetDrawMode(FPDF_PAGEOBJECT path,
                                         FPDF_DRAWMODE drawmode) {
  if (!path)
    return false;

  CPDF_PathObject* pPathObj = reinterpret_cast<CPDF_PathObject*>(path);
  if ((drawmode & FPDF_DRAWMODE::FillAlternate) &&
      (drawmode & FPDF_DRAWMODE::FillWinding)) {
    return false;
  }

  if (drawmode == FPDF_DRAWMODE::None)
    pPathObj->m_FillType = 0;
  else if (drawmode & FPDF_DRAWMODE::FillAlternate)
    pPathObj->m_FillType = FXFILL_ALTERNATE;
  else if (drawmode & FPDF_DRAWMODE::FillWinding)
    pPathObj->m_FillType = FXFILL_WINDING;
  pPathObj->m_bStroke = (drawmode & FPDF_DRAWMODE::Stroke);
  return true;
}
