// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_annot.h"

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfdoc/cpdf_annot.h"
#include "core/fpdfdoc/cpvt_color.h"
#include "core/fpdfdoc/cpvt_generateap.h"
#include "fpdfsdk/fsdk_define.h"

DLLEXPORT int STDCALL FPDFPage_GetAnnotCount(FPDF_PAGE page) {
  CPDF_Page* pPage = static_cast<CPDF_Page*>(page);
  if (!pPage || !pPage->m_pFormDict)
    return 0;
  CPDF_Array* pAnnots = pPage->m_pFormDict->GetArrayFor("Annots");
  return pAnnots ? (pAnnots->GetCount()) : 0;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPage_GetAnnot(FPDF_PAGE page,
                                              int index,
                                              void** annot) {
  // CPDF_Page* pPage = CPDFPageFromFPDFPage(page); // This gives me nullptr...
  CPDF_Page* pPage = static_cast<CPDF_Page*>(page);
  if (!pPage || !pPage->m_pFormDict)
    return false;
  CPDF_Array* pAnnots = pPage->m_pFormDict->GetArrayFor("Annots");
  if (!pAnnots || index < 0 ||
      static_cast<size_t>(index) >= pAnnots->GetCount())
    return false;

  // CPDF_Dictionary* pDict = ToDictionary(pAnnots->GetDirectObjectAt(index));
  // auto pAnnot = pdfium::MakeUnique<CPDF_Annot>(pDict,
  // pPage->m_pDocument.Get());
  CPDF_Dictionary* pDict = ToDictionary(pAnnots->GetDirectObjectAt(index));
  *annot = static_cast<FPDF_ANNOTATION>(pDict);
  return *annot ? true : false;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnotation_GetSubtype(FPDF_ANNOTATION annot,
                                                      char* buffer,
                                                      unsigned long bufLen) {
  CPDF_Dictionary* pAnnotDict = CPDFDictionaryFromFPDFAnnotation(annot);
  if (!pAnnotDict || !buffer)
    return false;
  CFX_ByteString subtype = pAnnotDict->GetStringFor("Subtype");
  unsigned long subtypeStringLen = subtype.GetLength() + 1;
  if (bufLen >= subtypeStringLen) {
    memcpy(buffer, subtype.c_str(), subtypeStringLen);
    return true;
  }
  return false;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnotation_GetColor(FPDF_ANNOTATION annot,
                                                    char* buffer,
                                                    unsigned long bufLen) {
  CPDF_Dictionary* pAnnotDict = CPDFDictionaryFromFPDFAnnotation(annot);
  if (!pAnnotDict || !buffer)
    return false;

  CPVT_Color defaultColor = CPVT_Color(CPVT_Color::kRGB, 0, 0, 0);
  if (pAnnotDict->GetStringFor("Subtype") == "Highlight")
    defaultColor = CPVT_Color(CPVT_Color::kRGB, 1, 1, 0);

  CFX_ByteString color =
      CPVT_GenerateAP::GetColorStringWithDefault(
          pAnnotDict->GetArrayFor("C"), defaultColor, PaintOperation::STROKE)
          .c_str();
  unsigned long colorStringLen = color.GetLength();
  if (bufLen >= colorStringLen) {
    memcpy(buffer, color.c_str(), colorStringLen);
    return true;
  }
  return false;
}

DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnotation_GetQuadPoints(FPDF_ANNOTATION annot,
                             FS_QUADPOINTSF* quadPoints) {
  CPDF_Dictionary* pAnnotDict = CPDFDictionaryFromFPDFAnnotation(annot);
  if (!quadPoints || !pAnnotDict)
    return false;
  CPDF_Array* pArray = pAnnotDict->GetArrayFor("QuadPoints");
  if (!pArray)
    return false;
  quadPoints->x1 = pArray->GetNumberAt(0);
  quadPoints->y1 = pArray->GetNumberAt(1);
  quadPoints->x2 = pArray->GetNumberAt(2);
  quadPoints->y2 = pArray->GetNumberAt(3);
  quadPoints->x3 = pArray->GetNumberAt(4);
  quadPoints->y3 = pArray->GetNumberAt(5);
  quadPoints->x4 = pArray->GetNumberAt(6);
  quadPoints->y4 = pArray->GetNumberAt(7);
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnotation_GetRect(FPDF_ANNOTATION annot,
                                                   FS_RECTF* rect) {
  CPDF_Dictionary* pAnnotDict = CPDFDictionaryFromFPDFAnnotation(annot);
  if (!rect || !pAnnotDict)
    return false;
  CFX_FloatRect rt = pAnnotDict->GetRectFor("Rect");
  rect->left = rt.left;
  rect->bottom = rt.bottom;
  rect->right = rt.right;
  rect->top = rt.top;
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnotation_GetContents(FPDF_ANNOTATION annot,
                                                       char* buffer,
                                                       unsigned long bufLen) {
  CPDF_Dictionary* pAnnotDict = CPDFDictionaryFromFPDFAnnotation(annot);
  if (!pAnnotDict || !buffer)
    return false;
  CFX_ByteString contents =
      pAnnotDict->GetUnicodeTextFor("Contents").UTF8Encode();
  unsigned long contentsStringLen = contents.GetLength();
  if (bufLen >= contentsStringLen) {
    memcpy(buffer, contents.c_str(), contentsStringLen);
    return true;
  }
  return false;
}

// TODO: add functions to extract: author, opacity, hidden, icon in closed state
