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
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage || !pPage->m_pFormDict)
    return 0;
  CPDF_Array* pAnnots = pPage->m_pFormDict->GetArrayFor("Annots");
  return pAnnots ? pAnnots->GetCount() : 0;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPage_GetAnnot(FPDF_PAGE page,
                                              int index,
                                              FPDF_ANNOTATION* annot) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage || !pPage->m_pFormDict)
    return false;
  CPDF_Array* pAnnots = pPage->m_pFormDict->GetArrayFor("Annots");
  if (!pAnnots || index < 0 ||
      static_cast<size_t>(index) >= pAnnots->GetCount()) {
    return false;
  }

  CPDF_Dictionary* pDict = ToDictionary(pAnnots->GetDirectObjectAt(index));
  *annot = FPDFAnnotationFromCPDFDictionary(pDict);
  return *annot ? true : false;
}

DLLEXPORT FPDF_SUBTYPE STDCALL
FPDFAnnotation_GetSubtype(FPDF_ANNOTATION annot) {
  CPDF_Dictionary* pAnnotDict = CPDFDictionaryFromFPDFAnnotation(annot);
  if (!pAnnotDict)
    return FPDF_ANNOT_UNKNOWN;
  CFX_ByteString sSubtype = pAnnotDict->GetStringFor("Subtype");
  if (sSubtype == "Text")
    return FPDF_ANNOT_TEXT;
  if (sSubtype == "Link")
    return FPDF_ANNOT_LINK;
  if (sSubtype == "FreeText")
    return FPDF_ANNOT_FREETEXT;
  if (sSubtype == "Line")
    return FPDF_ANNOT_LINE;
  if (sSubtype == "Square")
    return FPDF_ANNOT_SQUARE;
  if (sSubtype == "Circle")
    return FPDF_ANNOT_CIRCLE;
  if (sSubtype == "Polygon")
    return FPDF_ANNOT_POLYGON;
  if (sSubtype == "PolyLine")
    return FPDF_ANNOT_POLYLINE;
  if (sSubtype == "Highlight")
    return FPDF_ANNOT_HIGHLIGHT;
  if (sSubtype == "Underline")
    return FPDF_ANNOT_UNDERLINE;
  if (sSubtype == "Squiggly")
    return FPDF_ANNOT_SQUIGGLY;
  if (sSubtype == "StrikeOut")
    return FPDF_ANNOT_STRIKEOUT;
  if (sSubtype == "Stamp")
    return FPDF_ANNOT_STAMP;
  if (sSubtype == "Caret")
    return FPDF_ANNOT_CARET;
  if (sSubtype == "Ink")
    return FPDF_ANNOT_INK;
  if (sSubtype == "Popup")
    return FPDF_ANNOT_POPUP;
  if (sSubtype == "FileAttachment")
    return FPDF_ANNOT_FILEATTACHMENT;
  if (sSubtype == "Sound")
    return FPDF_ANNOT_SOUND;
  if (sSubtype == "Movie")
    return FPDF_ANNOT_MOVIE;
  if (sSubtype == "Widget")
    return FPDF_ANNOT_WIDGET;
  if (sSubtype == "Screen")
    return FPDF_ANNOT_SCREEN;
  if (sSubtype == "PrinterMark")
    return FPDF_ANNOT_PRINTERMARK;
  if (sSubtype == "TrapNet")
    return FPDF_ANNOT_TRAPNET;
  if (sSubtype == "Watermark")
    return FPDF_ANNOT_WATERMARK;
  if (sSubtype == "3D")
    return FPDF_ANNOT_THREED;
  if (sSubtype == "RichMedia")
    return FPDF_ANNOT_RICHMEDIA;
  if (sSubtype == "XFAWidget")
    return FPDF_ANNOT_XFAWIDGET;
  return FPDF_ANNOT_UNKNOWN;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnotation_GetColor(FPDF_ANNOTATION annot,
                                                    unsigned int* R,
                                                    unsigned int* G,
                                                    unsigned int* B,
                                                    unsigned int* A,
                                                    FPDFANNOT_COLORTYPE type) {
  CPDF_Dictionary* pAnnotDict = CPDFDictionaryFromFPDFAnnotation(annot);
  if (!pAnnotDict)
    return false;

  CFX_ByteString key;
  if (type == InteriorColor) {
    key = "IC";
  } else {
    key = "C";
  }
  CPDF_Array* pColor = pAnnotDict->GetArrayFor(key);
  if (pColor) {
    CPVT_Color color = CPVT_Color::ParseColor(*pColor);
    switch (color.nColorType) {
      case CPVT_Color::kRGB:
        *R = color.fColor1 * 255.f;
        *G = color.fColor2 * 255.f;
        *B = color.fColor3 * 255.f;
        break;
      case CPVT_Color::kGray:
        *R = 255.f * color.fColor1;
        *G = 255.f * color.fColor1;
        *B = 255.f * color.fColor1;
        break;
      case CPVT_Color::kCMYK:
        *R = 255.f * (1 - color.fColor1) * (1 - color.fColor4);
        *G = 255.f * (1 - color.fColor2) * (1 - color.fColor4);
        *B = 255.f * (1 - color.fColor3) * (1 - color.fColor4);
        break;
      case CPVT_Color::kTransparent:
        break;
    }
  } else {
    // Use default color.
    if (pAnnotDict->GetStringFor("Subtype") == "Highlight") {
      *R = 1;
      *G = 1;
      *B = 0;
    } else {
      *R = 0;
      *G = 0;
      *B = 0;
    }
  }
  *A =
      (pAnnotDict->KeyExist("CA") ? pAnnotDict->GetNumberFor("CA") : 1) * 255.f;
  return true;
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

DLLEXPORT FPDF_DWORD STDCALL FPDFAnnotation_GetText(FPDF_ANNOTATION annot,
                                                    char* buffer,
                                                    unsigned long bufLen,
                                                    FPDFANNOT_TEXTTYPE type) {
  CPDF_Dictionary* pAnnotDict = CPDFDictionaryFromFPDFAnnotation(annot);
  if (!pAnnotDict || !buffer)
    return 0;
  CFX_ByteString key;
  if (type == Author) {
    key = "T";
  } else {
    key = "Contents";
  }
  CFX_ByteString contents = pAnnotDict->GetUnicodeTextFor(key).UTF8Encode();
  unsigned long len = contents.GetLength();
  if (bufLen >= len + 1)
    memcpy(buffer, contents.c_str(), len + 1);
  return len;
}
