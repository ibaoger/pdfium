// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_annot.h"

#include <utility>

#include "core/fpdfapi/edit/cpdf_pagecontentgenerator.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfdoc/cpdf_annot.h"
#include "core/fpdfdoc/cpvt_color.h"
#include "core/fpdfdoc/cpvt_generateap.h"
#include "fpdfsdk/fsdk_define.h"

namespace {

// These checks ensure the consistency of annotation subtype values across core/
// and public.
static_assert(static_cast<int>(CPDF_Annot::Subtype::UNKNOWN) ==
                  FPDF_ANNOT_UNKNOWN,
              "CPDF_Annot::UNKNOWN value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::TEXT) == FPDF_ANNOT_TEXT,
              "CPDF_Annot::TEXT value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::LINK) == FPDF_ANNOT_LINK,
              "CPDF_Annot::LINK value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::FREETEXT) ==
                  FPDF_ANNOT_FREETEXT,
              "CPDF_Annot::FREETEXT value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::LINE) == FPDF_ANNOT_LINE,
              "CPDF_Annot::LINE value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::SQUARE) ==
                  FPDF_ANNOT_SQUARE,
              "CPDF_Annot::SQUARE value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::CIRCLE) ==
                  FPDF_ANNOT_CIRCLE,
              "CPDF_Annot::CIRCLE value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::POLYGON) ==
                  FPDF_ANNOT_POLYGON,
              "CPDF_Annot::POLYGON value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::POLYLINE) ==
                  FPDF_ANNOT_POLYLINE,
              "CPDF_Annot::POLYLINE value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::HIGHLIGHT) ==
                  FPDF_ANNOT_HIGHLIGHT,
              "CPDF_Annot::HIGHLIGHT value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::UNDERLINE) ==
                  FPDF_ANNOT_UNDERLINE,
              "CPDF_Annot::UNDERLINE value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::SQUIGGLY) ==
                  FPDF_ANNOT_SQUIGGLY,
              "CPDF_Annot::SQUIGGLY value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::STRIKEOUT) ==
                  FPDF_ANNOT_STRIKEOUT,
              "CPDF_Annot::STRIKEOUT value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::STAMP) == FPDF_ANNOT_STAMP,
              "CPDF_Annot::STAMP value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::CARET) == FPDF_ANNOT_CARET,
              "CPDF_Annot::CARET value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::INK) == FPDF_ANNOT_INK,
              "CPDF_Annot::INK value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::POPUP) == FPDF_ANNOT_POPUP,
              "CPDF_Annot::POPUP value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::FILEATTACHMENT) ==
                  FPDF_ANNOT_FILEATTACHMENT,
              "CPDF_Annot::FILEATTACHMENT value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::SOUND) == FPDF_ANNOT_SOUND,
              "CPDF_Annot::SOUND value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::MOVIE) == FPDF_ANNOT_MOVIE,
              "CPDF_Annot::MOVIE value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::WIDGET) ==
                  FPDF_ANNOT_WIDGET,
              "CPDF_Annot::WIDGET value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::SCREEN) ==
                  FPDF_ANNOT_SCREEN,
              "CPDF_Annot::SCREEN value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::PRINTERMARK) ==
                  FPDF_ANNOT_PRINTERMARK,
              "CPDF_Annot::PRINTERMARK value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::TRAPNET) ==
                  FPDF_ANNOT_TRAPNET,
              "CPDF_Annot::TRAPNET value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::WATERMARK) ==
                  FPDF_ANNOT_WATERMARK,
              "CPDF_Annot::WATERMARK value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::THREED) ==
                  FPDF_ANNOT_THREED,
              "CPDF_Annot::THREED value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::RICHMEDIA) ==
                  FPDF_ANNOT_RICHMEDIA,
              "CPDF_Annot::RICHMEDIA value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::XFAWIDGET) ==
                  FPDF_ANNOT_XFAWIDGET,
              "CPDF_Annot::XFAWIDGET value mismatch");

class CPDF_AnnotContext {
 public:
  CPDF_AnnotContext(CPDF_Dictionary* pAnnotDict,
                    CPDF_Page* pPage,
                    CPDF_Stream* pStream)
      : m_pAnnotDict(pAnnotDict), m_pPage(pPage) {
    SetForm(pStream);
  }
  ~CPDF_AnnotContext() {}

  bool HasForm() const { return !!m_pAnnotForm; }

  void SetForm(CPDF_Stream* pStream) {
    if (!pStream)
      return;

    // Reset the annotation matrix to be the identity matrix, since the
    // appearance stream already takes matrix into account.
    pStream->GetDict()->SetMatrixFor("Matrix",
                                     CFX_Matrix(1.f, 0.f, 0.f, 1.f, 0.f, 0.f));

    m_pAnnotForm = pdfium::MakeUnique<CPDF_Form>(
        m_pPage->m_pDocument.Get(), m_pPage->m_pResources.Get(), pStream);
    m_pAnnotForm->ParseContent(nullptr, nullptr, nullptr);
  }

  CPDF_Form* GetForm() const { return m_pAnnotForm.get(); }
  CPDF_Dictionary* GetAnnotDict() const { return m_pAnnotDict.Get(); }
  CPDF_Page* GetPage() const { return m_pPage.Get(); }

 private:
  std::unique_ptr<CPDF_Form> m_pAnnotForm;
  CFX_UnownedPtr<CPDF_Dictionary> m_pAnnotDict;
  CFX_UnownedPtr<CPDF_Page> m_pPage;
};

FPDF_ANNOTATION FPDFAnnotationFromCPDFAnnotContext(CPDF_AnnotContext* pAnnot) {
  return static_cast<FPDF_ANNOTATION>(pAnnot);
}

CPDF_AnnotContext* CPDFAnnotContextFromFPDFAnnotation(FPDF_ANNOTATION annot) {
  return static_cast<CPDF_AnnotContext*>(annot);
}

bool HasAPStream(CPDF_Dictionary* pAnnotDict) {
  return !!FPDFDOC_GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
}

}  // namespace

DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_IsSupportedSubtype(FPDF_ANNOTATION_SUBTYPE subtype) {
  return subtype == FPDF_ANNOT_CIRCLE || subtype == FPDF_ANNOT_HIGHLIGHT ||
         subtype == FPDF_ANNOT_INK || subtype == FPDF_ANNOT_POPUP ||
         subtype == FPDF_ANNOT_SQUARE || subtype == FPDF_ANNOT_SQUIGGLY ||
         subtype == FPDF_ANNOT_STRIKEOUT || subtype == FPDF_ANNOT_TEXT ||
         subtype == FPDF_ANNOT_UNDERLINE || subtype == FPDF_ANNOT_STAMP;
}

DLLEXPORT FPDF_BOOL STDCALL
FPDFPage_CreateAnnot(FPDF_PAGE page,
                     FPDF_ANNOTATION_SUBTYPE subtype,
                     FPDF_ANNOTATION* annot) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage || !FPDFAnnot_IsSupportedSubtype(subtype))
    return false;

  auto pDict = pdfium::MakeUnique<CPDF_Dictionary>(
      pPage->m_pDocument->GetByteStringPool());
  pDict->SetNewFor<CPDF_Name>("Type", "Annot");
  pDict->SetNewFor<CPDF_Name>("Subtype",
                              CPDF_Annot::AnnotSubtypeToString(
                                  static_cast<CPDF_Annot::Subtype>(subtype)));
  if (annot) {
    auto pNewAnnot =
        pdfium::MakeUnique<CPDF_AnnotContext>(pDict.get(), pPage, nullptr);
    *annot = FPDFAnnotationFromCPDFAnnotContext(pNewAnnot.release());
  }

  CPDF_Array* pAnnotList = pPage->m_pFormDict->GetArrayFor("Annots");
  if (!pAnnotList)
    pAnnotList = pPage->m_pFormDict->SetNewFor<CPDF_Array>("Annots");

  pAnnotList->Add(std::move(pDict));
  return true;
}

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
  if (!pPage || !pPage->m_pFormDict || index < 0 || !annot)
    return false;

  CPDF_Array* pAnnots = pPage->m_pFormDict->GetArrayFor("Annots");
  if (!pAnnots || static_cast<size_t>(index) >= pAnnots->GetCount())
    return false;

  CPDF_Dictionary* pDict = ToDictionary(pAnnots->GetDirectObjectAt(index));
  auto pNewAnnot = pdfium::MakeUnique<CPDF_AnnotContext>(pDict, pPage, nullptr);
  *annot = FPDFAnnotationFromCPDFAnnotContext(pNewAnnot.release());
  return true;
}

DLLEXPORT void STDCALL FPDFPage_CloseAnnot(FPDF_ANNOTATION annot) {
  delete CPDFAnnotContextFromFPDFAnnotation(annot);
}

DLLEXPORT FPDF_ANNOTATION_SUBTYPE STDCALL
FPDFAnnot_GetSubtype(FPDF_ANNOTATION annot) {
  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return FPDF_ANNOT_UNKNOWN;

  return static_cast<FPDF_ANNOTATION_SUBTYPE>(
      CPDF_Annot::StringToAnnotSubtype(pAnnotDict->GetStringFor("Subtype")));
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetPathObject(FPDF_ANNOTATION annot,
                                                    FPDF_PAGEOBJECT path) {
  CPDF_AnnotContext* pAnnot = CPDFAnnotContextFromFPDFAnnotation(annot);
  CPDF_PageObject* pPathObj = CPDFPageObjectFromFPDFPageObject(path);
  if (!pAnnot || !pPathObj)
    return false;

  CPDF_Dictionary* pAnnotDict = pAnnot->GetAnnotDict();
  CPDF_Page* pPage = pAnnot->GetPage();
  if (!pAnnotDict || !pPage)
    return false;

  // Check that the annotation type is supported by this method.
  FPDF_ANNOTATION_SUBTYPE subtype = FPDFAnnot_GetSubtype(annot);
  if (subtype != FPDF_ANNOT_INK && subtype != FPDF_ANNOT_STAMP)
    return false;

  // If the annotation does not have an AP stream yet, generate and set it.
  CFX_ByteString sExtGSDictName = "GS";
  CPDF_Stream* pStream = FPDFDOC_GetAnnotAP(pAnnot->GetAnnotDict(),
                                            CPDF_Annot::AppearanceMode::Normal);
  if (!pStream) {
    auto pExtGStateDict = CPVT_GenerateAP::GenerateExtGStateDict(
        *pAnnotDict, sExtGSDictName, "Normal");
    auto pResourceDict = CPVT_GenerateAP::GenerateResourceDict(
        pPage->m_pDocument.Get(), std::move(pExtGStateDict), nullptr);
    CFX_ByteTextBuf sStream;
    CPVT_GenerateAP::GenerateAndSetAPDict(pPage->m_pDocument.Get(), pAnnotDict,
                                          sStream, std::move(pResourceDict),
                                          false);
    pStream =
        FPDFDOC_GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
    if (!pStream)
      return false;
  }

  // Set up the annotation's corresponding form object for parsing its AP
  // stream.
  if (!pAnnot->HasForm())
    pAnnot->SetForm(pStream);

  CPDF_Form* pForm = pAnnot->GetForm();

  // If the path object did not come from the same annotation, then append it to
  // this annotation's object list.
  CPDF_PageObjectList* pObjList = pForm->GetPageObjectList();
  auto it = std::find_if(
      pObjList->begin(), pObjList->end(),
      [pPathObj](const std::unique_ptr<CPDF_PageObject>& candidate) {
        return candidate.get() == pPathObj;
      });
  if (it == pObjList->end()) {
    std::unique_ptr<CPDF_PageObject> pPageObjHolder(pPathObj);
    pObjList->push_back(std::move(pPageObjHolder));
  }

  // Generate the content stream data to be put into the annotation's AP stream.
  CPDF_PageContentGenerator generator(pPage);
  std::ostringstream buf;
  generator.ProcessFormObjects(&buf, pForm);

  // Retrieve the name for the graphics state dictionary within the AP stream.
  GraphicsData graphD;
  graphD.fillAlpha = pPathObj->m_GeneralState.GetFillAlpha();
  graphD.strokeAlpha = pPathObj->m_GeneralState.GetStrokeAlpha();
  graphD.blendType = pPathObj->m_GeneralState.GetBlendType();
  if (graphD.fillAlpha != 1.0f || graphD.strokeAlpha != 1.0f ||
      (graphD.blendType != FXDIB_BLEND_UNSUPPORTED &&
       graphD.blendType != FXDIB_BLEND_NORMAL)) {
    auto it = pPage->m_GraphicsMap.find(graphD);
    // When the page content generator processed this path, a dictionary name
    // should have been generated for the graphics state dictionary.
    if (it == pPage->m_GraphicsMap.end())
      return false;

    sExtGSDictName = PDF_NameEncode(it->second);
  }

  // Set up the graphics state diciontary within the AP stream.
  CPDF_Dictionary* pResourceDict = pStream->GetDict()->GetDictFor("Resources");
  if (!pResourceDict)
    pResourceDict = pStream->GetDict()->SetNewFor<CPDF_Dictionary>("Resources");

  CPDF_Dictionary* pExtGStateDict = pResourceDict->GetDictFor("ExtGState");
  if (!pExtGStateDict)
    pExtGStateDict = pResourceDict->SetNewFor<CPDF_Dictionary>("ExtGState");

  CPDF_Dictionary* pGS = pExtGStateDict->GetDictFor(sExtGSDictName);
  if (!pGS)
    pGS = pExtGStateDict->SetNewFor<CPDF_Dictionary>(sExtGSDictName);

  pGS->SetNewFor<CPDF_Number>("ca", graphD.fillAlpha);
  pGS->SetNewFor<CPDF_Number>("CA", graphD.strokeAlpha);
  if (graphD.blendType != FXDIB_BLEND_UNSUPPORTED)
    pGS->SetNewFor<CPDF_Name>("BM", pPathObj->m_GeneralState.GetBlendMode());

  // Set the content stream data in the AP stream.
  pStream->SetData(reinterpret_cast<const uint8_t*>(buf.str().c_str()),
                   buf.tellp());
  return true;
}

DLLEXPORT int STDCALL FPDFAnnot_GetPathObjectCount(FPDF_ANNOTATION annot) {
  CPDF_AnnotContext* pAnnot = CPDFAnnotContextFromFPDFAnnotation(annot);
  if (!pAnnot)
    return 0;

  if (!pAnnot->HasForm()) {
    CPDF_Stream* pStream = FPDFDOC_GetAnnotAP(
        pAnnot->GetAnnotDict(), CPDF_Annot::AppearanceMode::Normal);
    if (!pStream)
      return 0;

    pAnnot->SetForm(pStream);
  }

  int pathCount = 0;
  for (const auto& pObj : *pAnnot->GetForm()->GetPageObjectList()) {
    if (pObj && pObj->IsPath())
      ++pathCount;
  }
  return pathCount;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_GetPathObject(FPDF_ANNOTATION annot,
                                                    int index,
                                                    FPDF_PAGEOBJECT* path) {
  CPDF_AnnotContext* pAnnot = CPDFAnnotContextFromFPDFAnnotation(annot);
  if (!pAnnot || index < 0 || !path)
    return false;

  if (!pAnnot->HasForm()) {
    CPDF_Stream* pStream = FPDFDOC_GetAnnotAP(
        pAnnot->GetAnnotDict(), CPDF_Annot::AppearanceMode::Normal);
    if (!pStream)
      return false;

    pAnnot->SetForm(pStream);
  }

  // Retrieve the path object located at |index| in the list of path objects.
  // Note that the list of path objects is a sublist of the page object list,
  // consisting of only path objects specifically.
  int pathCount = -1;
  for (const auto& pObj : *pAnnot->GetForm()->GetPageObjectList()) {
    if (pObj && pObj->IsPath()) {
      ++pathCount;
      if (pathCount == index) {
        *path = FPDFPageObjectFromCPDFPageObject(pObj.get());
        return true;
      }
    }
  }
  return false;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetColor(FPDF_ANNOTATION annot,
                                               FPDFANNOT_COLORTYPE type,
                                               unsigned int R,
                                               unsigned int G,
                                               unsigned int B,
                                               unsigned int A) {
  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict || R > 255 || G > 255 || B > 255 || A > 255)
    return false;

  // For annotations with their apperance streams already defined, the path
  // stream' own color definitions take priority over the annotation color
  // definitions set by this method, hence this method will simply fail.
  if (HasAPStream(pAnnotDict))
    return false;

  // Set the opacity of the annotation.
  pAnnotDict->SetNewFor<CPDF_Number>("CA", A / 255.f);

  // Set the color of the annotation.
  CFX_ByteString key = type == FPDFANNOT_COLORTYPE_InteriorColor ? "IC" : "C";
  CPDF_Array* pColor = pAnnotDict->GetArrayFor(key);
  if (pColor)
    pColor->Clear();
  else
    pColor = pAnnotDict->SetNewFor<CPDF_Array>(key);

  pColor->AddNew<CPDF_Number>(R / 255.f);
  pColor->AddNew<CPDF_Number>(G / 255.f);
  pColor->AddNew<CPDF_Number>(B / 255.f);

  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_GetColor(FPDF_ANNOTATION annot,
                                               FPDFANNOT_COLORTYPE type,
                                               unsigned int* R,
                                               unsigned int* G,
                                               unsigned int* B,
                                               unsigned int* A) {
  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict || !R || !G || !B || !A)
    return false;

  // For annotations with their apperance streams already defined, the path
  // stream' own color definitions take priority over the annotation color
  // definitions retrieved by this method, hence this method will simply fail.
  if (HasAPStream(pAnnotDict))
    return false;

  CPDF_Array* pColor = pAnnotDict->GetArrayFor(
      type == FPDFANNOT_COLORTYPE_InteriorColor ? "IC" : "C");
  *A =
      (pAnnotDict->KeyExist("CA") ? pAnnotDict->GetNumberFor("CA") : 1) * 255.f;
  if (!pColor) {
    // Use default color. The default colors must be consistent with the ones
    // used to generate AP. See calls to GetColorStringWithDefault() in
    // CPVT_GenerateAP::Generate*AP().
    if (pAnnotDict->GetStringFor("Subtype") == "Highlight") {
      *R = 255;
      *G = 255;
      *B = 0;
    } else {
      *R = 0;
      *G = 0;
      *B = 0;
    }
    return true;
  }
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
      *R = 0;
      *G = 0;
      *B = 0;
      break;
  }
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_HasAttachmentPoints(FPDF_ANNOTATION annot) {
  if (!annot)
    return false;

  FPDF_ANNOTATION_SUBTYPE subtype = FPDFAnnot_GetSubtype(annot);
  return subtype == FPDF_ANNOT_LINK || subtype == FPDF_ANNOT_HIGHLIGHT ||
         subtype == FPDF_ANNOT_UNDERLINE || subtype == FPDF_ANNOT_SQUIGGLY ||
         subtype == FPDF_ANNOT_STRIKEOUT;
}

DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_SetAttachmentPoints(FPDF_ANNOTATION annot,
                              FS_QUADPOINTSF quadPoints) {
  if (!annot || !FPDFAnnot_HasAttachmentPoints(annot))
    return false;

  // Update the "QuadPoints" entry in the annotation dictionary.
  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();

  CPDF_Array* pQuadPoints = pAnnotDict->GetArrayFor("QuadPoints");
  if (pQuadPoints)
    pQuadPoints->Clear();
  else
    pQuadPoints = pAnnotDict->SetNewFor<CPDF_Array>("QuadPoints");

  pQuadPoints->AddNew<CPDF_Number>(quadPoints.x1);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints.y1);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints.x2);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints.y2);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints.x3);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints.y3);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints.x4);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints.y4);

  // If the annotation's appearance stream is defined, and the new quadpoints
  // defines a bigger bounding box than the appearance stream currently
  // specifies, then update the "BBox" entry in the AP dictionary too, since it
  // comes from annotation dictionary's "QuadPoints" entry.
  CPDF_Stream* pStream =
      FPDFDOC_GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
  if (pStream) {
    CFX_FloatRect newRect = CPDF_Annot::RectFromQuadPoints(pAnnotDict);
    if (newRect.Contains(pStream->GetDict()->GetRectFor("BBox")))
      pStream->GetDict()->SetRectFor("BBox", newRect);
  }
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_GetAttachmentPoints(FPDF_ANNOTATION annot,
                              FS_QUADPOINTSF* quadPoints) {
  if (!annot || !quadPoints || !FPDFAnnot_HasAttachmentPoints(annot))
    return false;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  CPDF_Array* pArray;

  // If the annotation's appearance stream is defined, then retrieve the
  // quadpoints defined by the "BBox" entry in the AP dictionary, since its
  // "BBox" entry comes from annotation dictionary's "QuadPoints" entry, but
  // takes priority over "QuadPoints" when rendering. Otherwise, retrieve
  // the "Quadpoints" entry from the annotation dictionary.
  CPDF_Stream* pStream =
      FPDFDOC_GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
  if (pStream) {
    pArray = pStream->GetDict()->GetArrayFor("BBox");
    if (!pArray)
      return false;

    // Convert the BBox array into quadpoint coordinates. BBox array follows the
    // order of a rectangle array: (left, bottom, right, up); and quadpoints
    // follows the following order: (top-left vertex, top-right vertex, bottom-
    // left vertex, bottom-right vertex).
    quadPoints->x1 = pArray->GetNumberAt(0);
    quadPoints->y1 = pArray->GetNumberAt(3);
    quadPoints->x2 = pArray->GetNumberAt(2);
    quadPoints->y2 = pArray->GetNumberAt(3);
    quadPoints->x3 = pArray->GetNumberAt(0);
    quadPoints->y3 = pArray->GetNumberAt(1);
    quadPoints->x4 = pArray->GetNumberAt(2);
    quadPoints->y4 = pArray->GetNumberAt(1);
  } else {
    pArray = pAnnotDict->GetArrayFor("QuadPoints");
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
  }
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetRect(FPDF_ANNOTATION annot,
                                              FS_RECTF rect) {
  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return false;

  CFX_FloatRect newRect(rect.left, rect.bottom, rect.right, rect.top);

  // Update the "Rect" entry in the annotation dictionary.
  pAnnotDict->SetRectFor("Rect", newRect);

  // If the annotation's appearance stream is defined, the annotation is of a
  // type that does not have quadpoints, and the new rectangle is bigger than
  // the current bounding box, then update the "BBox" entry in the AP
  // dictionary too, since its "BBox" entry comes from annotation dictionary's
  // "Rect" entry.
  if (FPDFAnnot_HasAttachmentPoints(annot))
    return true;

  CPDF_Stream* pStream =
      FPDFDOC_GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
  if (pStream && newRect.Contains(pStream->GetDict()->GetRectFor("BBox")))
    pStream->GetDict()->SetRectFor("BBox", newRect);

  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_GetRect(FPDF_ANNOTATION annot,
                                              FS_RECTF* rect) {
  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!rect || !pAnnotDict)
    return false;

  // If the annotation's appearance stream is defined and the annotation is of
  // a type that does not have quadpoints, then retrieve the rectangle defined
  // by the "BBox" entry in the AP dictionary, since its "BBox" entry comes
  // from annotation dictionary's "Rect" entry, but takes priority over "Rect"
  // when rendering. Otherwise, retrieve the "Rect" entry from the annotation
  // dictionary.
  CFX_FloatRect rt;
  CPDF_Stream* pStream =
      FPDFDOC_GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
  if (pStream && !FPDFAnnot_HasAttachmentPoints(annot))
    rt = pStream->GetDict()->GetRectFor("BBox");
  else
    rt = pAnnotDict->GetRectFor("Rect");

  if (rt.IsEmpty())
    return false;

  rect->left = rt.left;
  rect->bottom = rt.bottom;
  rect->right = rt.right;
  rect->top = rt.top;
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetText(FPDF_ANNOTATION annot,
                                              FPDFANNOT_TEXTTYPE type,
                                              FPDF_WIDESTRING text) {
  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return false;

  CFX_ByteString key = type == FPDFANNOT_TEXTTYPE_Author ? "T" : "Contents";
  FX_STRSIZE len = CFX_WideString::WStringLength(text);
  CFX_WideString encodedText = CFX_WideString::FromUTF16LE(text, len);
  pAnnotDict->SetNewFor<CPDF_String>(key, encodedText.UTF8Encode(), false);
  return true;
}

DLLEXPORT unsigned long STDCALL FPDFAnnot_GetText(FPDF_ANNOTATION annot,
                                                  FPDFANNOT_TEXTTYPE type,
                                                  void* buffer,
                                                  unsigned long buflen) {
  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return 0;

  CFX_ByteString key = type == FPDFANNOT_TEXTTYPE_Author ? "T" : "Contents";
  CFX_ByteString contents = pAnnotDict->GetUnicodeTextFor(key).UTF16LE_Encode();
  unsigned long len = contents.GetLength();
  if (buffer && buflen >= len)
    memcpy(buffer, contents.c_str(), len);

  return len;
}
