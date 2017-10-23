// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/Document.h"

#include <algorithm>
#include <sstream>
#include <utility>
#include <vector>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfdoc/cpdf_interform.h"
#include "core/fpdfdoc/cpdf_nametree.h"
#include "fpdfsdk/cpdfsdk_annotiteration.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_interform.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/javascript/Annot.h"
#include "fpdfsdk/javascript/Field.h"
#include "fpdfsdk/javascript/Icon.h"
#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_EventHandler.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/app.h"
#include "fpdfsdk/javascript/cjs_event_context.h"
#include "fpdfsdk/javascript/cjs_runtime.h"
#include "fpdfsdk/javascript/resource.h"
#include "third_party/base/numerics/safe_math.h"
#include "third_party/base/ptr_util.h"

JSConstSpec CJS_PrintParamsObj::ConstSpecs[] = {{0, JSConstSpec::Number, 0, 0}};

JSPropertySpec CJS_PrintParamsObj::PropertySpecs[] = {{0, 0, 0}};

JSMethodSpec CJS_PrintParamsObj::MethodSpecs[] = {{0, 0}};

IMPLEMENT_JS_CLASS(CJS_PrintParamsObj, PrintParamsObj, PrintParamsObj)

PrintParamsObj::PrintParamsObj(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject) {
  bUI = true;
  nStart = 0;
  nEnd = 0;
  bSilent = false;
  bShrinkToFit = false;
  bPrintAsImage = false;
  bReverse = false;
  bAnnotations = true;
}

#define MINWIDTH 5.0f
#define MINHEIGHT 5.0f

JSConstSpec CJS_Document::ConstSpecs[] = {{0, JSConstSpec::Number, 0, 0}};

JSPropertySpec CJS_Document::PropertySpecs[] = {
    {"ADBE", get_ADBE_static, set_ADBE_static},
    {"author", get_author_static, set_author_static},
    {"baseURL", get_base_URL_static, set_base_URL_static},
    {"bookmarkRoot", get_bookmark_root_static, set_bookmark_root_static},
    {"calculate", get_calculate_static, set_calculate_static},
    {"Collab", get_collab_static, set_collab_static},
    {"creationDate", get_creation_date_static, set_creation_date_static},
    {"creator", get_creator_static, set_creator_static},
    {"delay", get_delay_static, set_delay_static},
    {"dirty", get_dirty_static, set_dirty_static},
    {"documentFileName", get_document_file_name_static,
     set_document_file_name_static},
    {"external", get_external_static, set_external_static},
    {"filesize", get_filesize_static, set_filesize_static},
    {"icons", get_icons_static, set_icons_static},
    {"info", get_info_static, set_info_static},
    {"keywords", get_keywords_static, set_keywords_static},
    {"layout", get_layout_static, set_layout_static},
    {"media", get_media_static, set_media_static},
    {"modDate", get_mod_date_static, set_mod_date_static},
    {"mouseX", get_mouse_x_static, set_mouse_x_static},
    {"mouseY", get_mouse_y_static, set_mouse_y_static},
    {"numFields", get_num_fields_static, set_num_fields_static},
    {"numPages", get_num_pages_static, set_num_pages_static},
    {"pageNum", get_page_num_static, set_page_num_static},
    {"pageWindowRect", get_page_window_rect_static,
     set_page_window_rect_static},
    {"path", get_path_static, set_path_static},
    {"producer", get_producer_static, set_producer_static},
    {"subject", get_subject_static, set_subject_static},
    {"title", get_title_static, set_title_static},
    {"URL", get_URL_static, set_URL_static},
    {"zoom", get_zoom_static, set_zoom_static},
    {"zoomType", get_zoom_type_static, set_zoom_type_static},
    {0, 0, 0}};

JSMethodSpec CJS_Document::MethodSpecs[] = {
    {"addAnnot", addAnnot_static},
    {"addField", addField_static},
    {"addLink", addLink_static},
    {"addIcon", addIcon_static},
    {"calculateNow", calculateNow_static},
    {"closeDoc", closeDoc_static},
    {"createDataObject", createDataObject_static},
    {"deletePages", deletePages_static},
    {"exportAsText", exportAsText_static},
    {"exportAsFDF", exportAsFDF_static},
    {"exportAsXFDF", exportAsXFDF_static},
    {"extractPages", extractPages_static},
    {"getAnnot", getAnnot_static},
    {"getAnnots", getAnnots_static},
    {"getAnnot3D", getAnnot3D_static},
    {"getAnnots3D", getAnnots3D_static},
    {"getField", getField_static},
    {"getIcon", getIcon_static},
    {"getLinks", getLinks_static},
    {"getNthFieldName", getNthFieldName_static},
    {"getOCGs", getOCGs_static},
    {"getPageBox", getPageBox_static},
    {"getPageNthWord", getPageNthWord_static},
    {"getPageNthWordQuads", getPageNthWordQuads_static},
    {"getPageNumWords", getPageNumWords_static},
    {"getPrintParams", getPrintParams_static},
    {"getURL", getURL_static},
    {"gotoNamedDest", gotoNamedDest_static},
    {"importAnFDF", importAnFDF_static},
    {"importAnXFDF", importAnXFDF_static},
    {"importTextData", importTextData_static},
    {"insertPages", insertPages_static},
    {"mailForm", mailForm_static},
    {"print", print_static},
    {"removeField", removeField_static},
    {"replacePages", replacePages_static},
    {"resetForm", resetForm_static},
    {"removeIcon", removeIcon_static},
    {"saveAs", saveAs_static},
    {"submitForm", submitForm_static},
    {"syncAnnotScan", syncAnnotScan_static},
    {"mailDoc", mailDoc_static},
    {0, 0}};

IMPLEMENT_JS_CLASS(CJS_Document, Document, Document)

void CJS_Document::InitInstance(IJS_Runtime* pIRuntime) {
  CJS_Runtime* pRuntime = static_cast<CJS_Runtime*>(pIRuntime);
  Document* pDoc = static_cast<Document*>(GetEmbedObject());
  pDoc->SetFormFillEnv(pRuntime->GetFormFillEnv());
}

Document::Document(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject),
      m_pFormFillEnv(nullptr),
      m_cwBaseURL(L""),
      m_bDelay(false) {}

Document::~Document() {}

// the total number of fields in document.
bool Document::get_num_fields(CJS_Runtime* pRuntime,
                              CJS_Value* vp,
                              WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  CPDF_InterForm* pPDFForm = pInterForm->GetInterForm();
  vp->Set(pRuntime, static_cast<int>(pPDFForm->CountFields(WideString())));
  return true;
}

bool Document::set_num_fields(CJS_Runtime* pRuntime,
                              const CJS_Value& vp,
                              WideString* sError) {
  *sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
  return false;
}

bool Document::get_dirty(CJS_Runtime* pRuntime,
                         CJS_Value* vp,
                         WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  vp->Set(pRuntime, !!m_pFormFillEnv->GetChangeMark());
  return true;
}

bool Document::set_dirty(CJS_Runtime* pRuntime,
                         const CJS_Value& vp,
                         WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  vp.ToBool(pRuntime) ? m_pFormFillEnv->SetChangeMark()
                      : m_pFormFillEnv->ClearChangeMark();

  return true;
}

bool Document::get_ADBE(CJS_Runtime* pRuntime,
                        CJS_Value* vp,
                        WideString* sError) {
  vp->SetNull(pRuntime);
  return true;
}

bool Document::set_ADBE(CJS_Runtime* pRuntime,
                        const CJS_Value& vp,
                        WideString* sError) {
  return true;
}

bool Document::get_page_num(CJS_Runtime* pRuntime,
                            CJS_Value* vp,
                            WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  if (CPDFSDK_PageView* pPageView = m_pFormFillEnv->GetCurrentView())
    vp->Set(pRuntime, pPageView->GetPageIndex());
  return true;
}

bool Document::set_page_num(CJS_Runtime* pRuntime,
                            const CJS_Value& vp,
                            WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  int iPageCount = m_pFormFillEnv->GetPageCount();
  int iPageNum = vp.ToInt(pRuntime);
  if (iPageNum >= 0 && iPageNum < iPageCount)
    m_pFormFillEnv->JS_docgotoPage(iPageNum);
  else if (iPageNum >= iPageCount)
    m_pFormFillEnv->JS_docgotoPage(iPageCount - 1);
  else if (iPageNum < 0)
    m_pFormFillEnv->JS_docgotoPage(0);

  return true;
}

pdfium::Optional<CJS_Value> Document::addAnnot(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Not supported.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::addField(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Not supported.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::exportAsText(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Unsafe, not supported.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::exportAsFDF(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Unsafe, not supported.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::exportAsXFDF(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Unsafe, not supported.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::getField(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  if (params.size() < 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return pdfium::Optional<CJS_Value>();
  }
  if (!m_pFormFillEnv) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return pdfium::Optional<CJS_Value>();
  }

  WideString wideName = params[0].ToWideString(pRuntime);
  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  CPDF_InterForm* pPDFForm = pInterForm->GetInterForm();
  if (pPDFForm->CountFields(wideName) <= 0) {
    CJS_Value null(pRuntime);
    null.SetNull(pRuntime);
    return pdfium::Optional<CJS_Value>(null);
  }

  v8::Local<v8::Object> pFieldObj =
      pRuntime->NewFxDynamicObj(CJS_Field::g_nObjDefnID);
  if (pFieldObj.IsEmpty())
    return pdfium::Optional<CJS_Value>();

  CJS_Field* pJSField =
      static_cast<CJS_Field*>(pRuntime->GetObjectPrivate(pFieldObj));
  Field* pField = static_cast<Field*>(pJSField->GetEmbedObject());
  pField->AttachField(this, wideName);

  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime, pJSField));
}

// Gets the name of the nth field in the document
pdfium::Optional<CJS_Value> Document::getNthFieldName(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  if (params.size() != 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return pdfium::Optional<CJS_Value>();
  }
  if (!m_pFormFillEnv) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return pdfium::Optional<CJS_Value>();
  }

  int nIndex = params[0].ToInt(pRuntime);
  if (nIndex < 0) {
    sError = JSGetStringFromID(IDS_STRING_JSVALUEERROR);
    return pdfium::Optional<CJS_Value>();
  }

  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  CPDF_InterForm* pPDFForm = pInterForm->GetInterForm();
  CPDF_FormField* pField = pPDFForm->GetField(nIndex, WideString());
  if (!pField)
    return pdfium::Optional<CJS_Value>();

  return pdfium::Optional<CJS_Value>(
      CJS_Value(pRuntime, pField->GetFullName().c_str()));
}

pdfium::Optional<CJS_Value> Document::importAnFDF(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Unsafe, not supported.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::importAnXFDF(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Unsafe, not supported.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::importTextData(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Unsafe, not supported.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

// exports the form data and mails the resulting fdf file as an attachment to
// all recipients.
// comment: need reader supports
pdfium::Optional<CJS_Value> Document::mailForm(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  if (!m_pFormFillEnv) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return pdfium::Optional<CJS_Value>();
  }
  if (!m_pFormFillEnv->GetPermissions(FPDFPERM_EXTRACT_ACCESS)) {
    sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
    return pdfium::Optional<CJS_Value>();
  }

  int iLength = params.size();
  bool bUI = iLength > 0 ? params[0].ToBool(pRuntime) : true;
  WideString cTo = iLength > 1 ? params[1].ToWideString(pRuntime) : L"";
  WideString cCc = iLength > 2 ? params[2].ToWideString(pRuntime) : L"";
  WideString cBcc = iLength > 3 ? params[3].ToWideString(pRuntime) : L"";
  WideString cSubject = iLength > 4 ? params[4].ToWideString(pRuntime) : L"";
  WideString cMsg = iLength > 5 ? params[5].ToWideString(pRuntime) : L"";

  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  ByteString sTextBuf = pInterForm->ExportFormToFDFTextBuf();
  if (sTextBuf.GetLength() == 0)
    return pdfium::Optional<CJS_Value>();

  size_t nBufSize = sTextBuf.GetLength();
  char* pMutableBuf = FX_Alloc(char, nBufSize);
  memcpy(pMutableBuf, sTextBuf.c_str(), nBufSize);

  pRuntime->BeginBlock();
  CPDFSDK_FormFillEnvironment* pFormFillEnv = pRuntime->GetFormFillEnv();
  pFormFillEnv->JS_docmailForm(pMutableBuf, nBufSize, bUI, cTo.c_str(),
                               cSubject.c_str(), cCc.c_str(), cBcc.c_str(),
                               cMsg.c_str());
  pRuntime->EndBlock();
  FX_Free(pMutableBuf);

  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::print(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  if (!m_pFormFillEnv) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return pdfium::Optional<CJS_Value>();
  }

  bool bUI = true;
  int nStart = 0;
  int nEnd = 0;
  bool bSilent = false;
  bool bShrinkToFit = false;
  bool bPrintAsImage = false;
  bool bReverse = false;
  bool bAnnotations = false;
  int nlength = params.size();
  if (nlength == 9) {
    if (params[8].GetType() == CJS_Value::VT_object) {
      v8::Local<v8::Object> pObj = params[8].ToV8Object(pRuntime);
      if (CFXJS_Engine::GetObjDefnID(pObj) ==
          CJS_PrintParamsObj::g_nObjDefnID) {
        if (CJS_Object* pJSObj = params[8].ToObject(pRuntime)) {
          if (PrintParamsObj* pprintparamsObj =
                  static_cast<PrintParamsObj*>(pJSObj->GetEmbedObject())) {
            bUI = pprintparamsObj->bUI;
            nStart = pprintparamsObj->nStart;
            nEnd = pprintparamsObj->nEnd;
            bSilent = pprintparamsObj->bSilent;
            bShrinkToFit = pprintparamsObj->bShrinkToFit;
            bPrintAsImage = pprintparamsObj->bPrintAsImage;
            bReverse = pprintparamsObj->bReverse;
            bAnnotations = pprintparamsObj->bAnnotations;
          }
        }
      }
    }
  } else {
    if (nlength >= 1)
      bUI = params[0].ToBool(pRuntime);
    if (nlength >= 2)
      nStart = params[1].ToInt(pRuntime);
    if (nlength >= 3)
      nEnd = params[2].ToInt(pRuntime);
    if (nlength >= 4)
      bSilent = params[3].ToBool(pRuntime);
    if (nlength >= 5)
      bShrinkToFit = params[4].ToBool(pRuntime);
    if (nlength >= 6)
      bPrintAsImage = params[5].ToBool(pRuntime);
    if (nlength >= 7)
      bReverse = params[6].ToBool(pRuntime);
    if (nlength >= 8)
      bAnnotations = params[7].ToBool(pRuntime);
  }

  if (m_pFormFillEnv) {
    m_pFormFillEnv->JS_docprint(bUI, nStart, nEnd, bSilent, bShrinkToFit,
                                bPrintAsImage, bReverse, bAnnotations);
    return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
  }

  return pdfium::Optional<CJS_Value>();
}

// removes the specified field from the document.
// comment:
// note: if the filed name is not rational, adobe is dumb for it.

pdfium::Optional<CJS_Value> Document::removeField(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  if (params.size() != 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return pdfium::Optional<CJS_Value>();
  }
  if (!m_pFormFillEnv) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return pdfium::Optional<CJS_Value>();
  }
  if (!(m_pFormFillEnv->GetPermissions(FPDFPERM_MODIFY) ||
        m_pFormFillEnv->GetPermissions(FPDFPERM_ANNOT_FORM))) {
    sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
    return pdfium::Optional<CJS_Value>();
  }
  WideString sFieldName = params[0].ToWideString(pRuntime);
  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  std::vector<CPDFSDK_Annot::ObservedPtr> widgets;
  pInterForm->GetWidgets(sFieldName, &widgets);
  if (widgets.empty())
    return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));

  for (const auto& pAnnot : widgets) {
    CPDFSDK_Widget* pWidget = static_cast<CPDFSDK_Widget*>(pAnnot.Get());
    if (!pWidget)
      continue;

    CFX_FloatRect rcAnnot = pWidget->GetRect();
    --rcAnnot.left;
    --rcAnnot.bottom;
    ++rcAnnot.right;
    ++rcAnnot.top;

    std::vector<CFX_FloatRect> aRefresh(1, rcAnnot);
    UnderlyingPageType* pPage = pWidget->GetUnderlyingPage();
    ASSERT(pPage);

    // If there is currently no pageview associated with the page being used
    // do not create one. We may be in the process of tearing down the document
    // and creating a new pageview at this point will cause bad things.
    CPDFSDK_PageView* pPageView = m_pFormFillEnv->GetPageView(pPage, false);
    if (pPageView) {
#if PDF_ENABLE_XFA
      pPageView->DeleteAnnot(pWidget);
#endif  // PDF_ENABLE_XFA
      pPageView->UpdateRects(aRefresh);
    }
  }
  m_pFormFillEnv->SetChangeMark();

  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

// reset filed values within a document.
// comment:
// note: if the fields names r not rational, aodbe is dumb for it.

pdfium::Optional<CJS_Value> Document::resetForm(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  if (!m_pFormFillEnv) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return pdfium::Optional<CJS_Value>();
  }
  if (!(m_pFormFillEnv->GetPermissions(FPDFPERM_MODIFY) ||
        m_pFormFillEnv->GetPermissions(FPDFPERM_ANNOT_FORM) ||
        m_pFormFillEnv->GetPermissions(FPDFPERM_FILL_FORM))) {
    sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
    return pdfium::Optional<CJS_Value>();
  }

  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  CPDF_InterForm* pPDFForm = pInterForm->GetInterForm();
  CJS_Array aName;

  if (params.empty()) {
    pPDFForm->ResetForm(true);
    m_pFormFillEnv->SetChangeMark();
    return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
  }

  switch (params[0].GetType()) {
    default:
      aName = CJS_Array(params[0].ToV8Array(pRuntime));
      break;
    case CJS_Value::VT_string:
      aName.SetElement(pRuntime, 0, params[0]);
      break;
  }

  std::vector<CPDF_FormField*> aFields;
  for (int i = 0, isz = aName.GetLength(pRuntime); i < isz; ++i) {
    CJS_Value valElement = aName.GetElement(pRuntime, i);
    WideString swVal = valElement.ToWideString(pRuntime);
    for (int j = 0, jsz = pPDFForm->CountFields(swVal); j < jsz; ++j)
      aFields.push_back(pPDFForm->GetField(j, swVal));
  }

  if (!aFields.empty()) {
    pPDFForm->ResetForm(aFields, true, true);
    m_pFormFillEnv->SetChangeMark();
  }

  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::saveAs(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Unsafe, not supported.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::syncAnnotScan(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::submitForm(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  int nSize = params.size();
  if (nSize < 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return pdfium::Optional<CJS_Value>();
  }
  if (!m_pFormFillEnv) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return pdfium::Optional<CJS_Value>();
  }

  CJS_Array aFields;
  WideString strURL;
  bool bFDF = true;
  bool bEmpty = false;
  CJS_Value v = params[0];
  if (v.GetType() == CJS_Value::VT_string) {
    strURL = params[0].ToWideString(pRuntime);
    if (nSize > 1)
      bFDF = params[1].ToBool(pRuntime);
    if (nSize > 2)
      bEmpty = params[2].ToBool(pRuntime);
    if (nSize > 3)
      aFields = CJS_Array(params[3].ToV8Array(pRuntime));
  } else if (v.GetType() == CJS_Value::VT_object) {
    v8::Local<v8::Object> pObj = params[0].ToV8Object(pRuntime);
    v8::Local<v8::Value> pValue = pRuntime->GetObjectProperty(pObj, L"cURL");
    if (!pValue.IsEmpty())
      strURL = CJS_Value(pRuntime, pValue).ToWideString(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"bFDF");
    bFDF = CJS_Value(pRuntime, pValue).ToBool(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"bEmpty");
    bEmpty = CJS_Value(pRuntime, pValue).ToBool(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"aFields");
    aFields = CJS_Array(CJS_Value(pRuntime, pValue).ToV8Array(pRuntime));
  }

  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  CPDF_InterForm* pPDFInterForm = pInterForm->GetInterForm();
  if (aFields.GetLength(pRuntime) == 0 && bEmpty) {
    if (pPDFInterForm->CheckRequiredFields(nullptr, true)) {
      pRuntime->BeginBlock();
      pInterForm->SubmitForm(strURL, false);
      pRuntime->EndBlock();
    }
    return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
  }

  std::vector<CPDF_FormField*> fieldObjects;
  for (int i = 0, sz = aFields.GetLength(pRuntime); i < sz; ++i) {
    CJS_Value valName = aFields.GetElement(pRuntime, i);
    WideString sName = valName.ToWideString(pRuntime);
    CPDF_InterForm* pPDFForm = pInterForm->GetInterForm();
    for (int j = 0, jsz = pPDFForm->CountFields(sName); j < jsz; ++j) {
      CPDF_FormField* pField = pPDFForm->GetField(j, sName);
      if (!bEmpty && pField->GetValue().IsEmpty())
        continue;

      fieldObjects.push_back(pField);
    }
  }

  if (pPDFInterForm->CheckRequiredFields(&fieldObjects, true)) {
    pRuntime->BeginBlock();
    pInterForm->SubmitFields(strURL, fieldObjects, true, !bFDF);
    pRuntime->EndBlock();
  }
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

void Document::SetFormFillEnv(CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  m_pFormFillEnv.Reset(pFormFillEnv);
}

bool Document::get_bookmark_root(CJS_Runtime* pRuntime,
                                 CJS_Value* vp,
                                 WideString* sError) {
  return true;
}

bool Document::set_bookmark_root(CJS_Runtime* pRuntime,
                                 const CJS_Value& vp,
                                 WideString* sError) {
  return true;
}

pdfium::Optional<CJS_Value> Document::mailDoc(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // TODO(tsepez): Check maximum number of allowed params.
  bool bUI = true;
  WideString cTo = L"";
  WideString cCc = L"";
  WideString cBcc = L"";
  WideString cSubject = L"";
  WideString cMsg = L"";

  if (params.size() >= 1)
    bUI = params[0].ToBool(pRuntime);
  if (params.size() >= 2)
    cTo = params[1].ToWideString(pRuntime);
  if (params.size() >= 3)
    cCc = params[2].ToWideString(pRuntime);
  if (params.size() >= 4)
    cBcc = params[3].ToWideString(pRuntime);
  if (params.size() >= 5)
    cSubject = params[4].ToWideString(pRuntime);
  if (params.size() >= 6)
    cMsg = params[5].ToWideString(pRuntime);

  if (params.size() >= 1 && params[0].GetType() == CJS_Value::VT_object) {
    v8::Local<v8::Object> pObj = params[0].ToV8Object(pRuntime);

    v8::Local<v8::Value> pValue = pRuntime->GetObjectProperty(pObj, L"bUI");
    bUI = CJS_Value(pRuntime, pValue).ToBool(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"cTo");
    cTo = CJS_Value(pRuntime, pValue).ToWideString(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"cCc");
    cCc = CJS_Value(pRuntime, pValue).ToWideString(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"cBcc");
    cBcc = CJS_Value(pRuntime, pValue).ToWideString(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"cSubject");
    cSubject = CJS_Value(pRuntime, pValue).ToWideString(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"cMsg");
    cMsg = CJS_Value(pRuntime, pValue).ToWideString(pRuntime);
  }

  pRuntime->BeginBlock();
  CPDFSDK_FormFillEnvironment* pFormFillEnv = pRuntime->GetFormFillEnv();
  pFormFillEnv->JS_docmailForm(nullptr, 0, bUI, cTo.c_str(), cSubject.c_str(),
                               cCc.c_str(), cBcc.c_str(), cMsg.c_str());
  pRuntime->EndBlock();
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

bool Document::get_author(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError) {
  return getPropertyInternal(pRuntime, vp, "Author", sError);
}

bool Document::set_author(CJS_Runtime* pRuntime,
                          const CJS_Value& vp,
                          WideString* sError) {
  return setPropertyInternal(pRuntime, vp, "Author", sError);
}

bool Document::get_info(CJS_Runtime* pRuntime,
                        CJS_Value* vp,
                        WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  const auto* pDictionary = m_pFormFillEnv->GetPDFDocument()->GetInfo();
  if (!pDictionary)
    return false;

  WideString cwAuthor = pDictionary->GetUnicodeTextFor("Author");
  WideString cwTitle = pDictionary->GetUnicodeTextFor("Title");
  WideString cwSubject = pDictionary->GetUnicodeTextFor("Subject");
  WideString cwKeywords = pDictionary->GetUnicodeTextFor("Keywords");
  WideString cwCreator = pDictionary->GetUnicodeTextFor("Creator");
  WideString cwProducer = pDictionary->GetUnicodeTextFor("Producer");
  WideString cwCreationDate = pDictionary->GetUnicodeTextFor("CreationDate");
  WideString cwModDate = pDictionary->GetUnicodeTextFor("ModDate");
  WideString cwTrapped = pDictionary->GetUnicodeTextFor("Trapped");

  v8::Local<v8::Object> pObj = pRuntime->NewFxDynamicObj(-1);
  pRuntime->PutObjectProperty(pObj, L"Author",
                              pRuntime->NewString(cwAuthor.AsStringView()));
  pRuntime->PutObjectProperty(pObj, L"Title",
                              pRuntime->NewString(cwTitle.AsStringView()));
  pRuntime->PutObjectProperty(pObj, L"Subject",
                              pRuntime->NewString(cwSubject.AsStringView()));
  pRuntime->PutObjectProperty(pObj, L"Keywords",
                              pRuntime->NewString(cwKeywords.AsStringView()));
  pRuntime->PutObjectProperty(pObj, L"Creator",
                              pRuntime->NewString(cwCreator.AsStringView()));
  pRuntime->PutObjectProperty(pObj, L"Producer",
                              pRuntime->NewString(cwProducer.AsStringView()));
  pRuntime->PutObjectProperty(
      pObj, L"CreationDate",
      pRuntime->NewString(cwCreationDate.AsStringView()));
  pRuntime->PutObjectProperty(pObj, L"ModDate",
                              pRuntime->NewString(cwModDate.AsStringView()));
  pRuntime->PutObjectProperty(pObj, L"Trapped",
                              pRuntime->NewString(cwTrapped.AsStringView()));

  // It's to be compatible to non-standard info dictionary.
  for (const auto& it : *pDictionary) {
    const ByteString& bsKey = it.first;
    CPDF_Object* pValueObj = it.second.get();
    WideString wsKey = WideString::FromUTF8(bsKey.AsStringView());
    if (pValueObj->IsString() || pValueObj->IsName()) {
      pRuntime->PutObjectProperty(
          pObj, wsKey,
          pRuntime->NewString(pValueObj->GetUnicodeText().AsStringView()));
    } else if (pValueObj->IsNumber()) {
      pRuntime->PutObjectProperty(pObj, wsKey,
                                  pRuntime->NewNumber(pValueObj->GetNumber()));
    } else if (pValueObj->IsBoolean()) {
      pRuntime->PutObjectProperty(
          pObj, wsKey, pRuntime->NewBoolean(!!pValueObj->GetInteger()));
    }
  }
  vp->Set(pRuntime, pObj);
  return true;
}

bool Document::set_info(CJS_Runtime* pRuntime,
                        const CJS_Value& vp,
                        WideString* sError) {
  *sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
  return false;
}

bool Document::getPropertyInternal(CJS_Runtime* pRuntime,
                                   CJS_Value* vp,
                                   const ByteString& propName,
                                   WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  CPDF_Dictionary* pDictionary = m_pFormFillEnv->GetPDFDocument()->GetInfo();
  if (!pDictionary)
    return false;

  vp->Set(pRuntime, pDictionary->GetUnicodeTextFor(propName));
  return true;
}

bool Document::setPropertyInternal(CJS_Runtime* pRuntime,
                                   const CJS_Value& vp,
                                   const ByteString& propName,
                                   WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  CPDF_Dictionary* pDictionary = m_pFormFillEnv->GetPDFDocument()->GetInfo();
  if (!pDictionary)
    return false;

  if (!m_pFormFillEnv->GetPermissions(FPDFPERM_MODIFY)) {
    *sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
    return false;
  }
  WideString csProperty = vp.ToWideString(pRuntime);
  pDictionary->SetNewFor<CPDF_String>(propName, PDF_EncodeText(csProperty),
                                      false);
  m_pFormFillEnv->SetChangeMark();
  return true;
}

bool Document::get_creation_date(CJS_Runtime* pRuntime,
                                 CJS_Value* vp,
                                 WideString* sError) {
  return getPropertyInternal(pRuntime, vp, "CreationDate", sError);
}

bool Document::set_creation_date(CJS_Runtime* pRuntime,
                                 const CJS_Value& vp,
                                 WideString* sError) {
  return setPropertyInternal(pRuntime, vp, "CreationDate", sError);
}

bool Document::get_creator(CJS_Runtime* pRuntime,
                           CJS_Value* vp,
                           WideString* sError) {
  return getPropertyInternal(pRuntime, vp, "Creator", sError);
}

bool Document::set_creator(CJS_Runtime* pRuntime,
                           const CJS_Value& vp,
                           WideString* sError) {
  return setPropertyInternal(pRuntime, vp, "Creator", sError);
}

bool Document::get_delay(CJS_Runtime* pRuntime,
                         CJS_Value* vp,
                         WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }
  vp->Set(pRuntime, m_bDelay);
  return true;
}

bool Document::set_delay(CJS_Runtime* pRuntime,
                         const CJS_Value& vp,
                         WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  if (!m_pFormFillEnv->GetPermissions(FPDFPERM_MODIFY)) {
    *sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
    return false;
  }

  m_bDelay = vp.ToBool(pRuntime);
  if (m_bDelay) {
    m_DelayData.clear();
    return true;
  }

  std::list<std::unique_ptr<CJS_DelayData>> DelayDataToProcess;
  DelayDataToProcess.swap(m_DelayData);
  for (const auto& pData : DelayDataToProcess)
    Field::DoDelay(m_pFormFillEnv.Get(), pData.get());

  return true;
}

bool Document::get_keywords(CJS_Runtime* pRuntime,
                            CJS_Value* vp,
                            WideString* sError) {
  return getPropertyInternal(pRuntime, vp, "Keywords", sError);
}

bool Document::set_keywords(CJS_Runtime* pRuntime,
                            const CJS_Value& vp,
                            WideString* sError) {
  return setPropertyInternal(pRuntime, vp, "Keywords", sError);
}

bool Document::get_mod_date(CJS_Runtime* pRuntime,
                            CJS_Value* vp,
                            WideString* sError) {
  return getPropertyInternal(pRuntime, vp, "ModDate", sError);
}

bool Document::set_mod_date(CJS_Runtime* pRuntime,
                            const CJS_Value& vp,
                            WideString* sError) {
  return setPropertyInternal(pRuntime, vp, "ModDate", sError);
}

bool Document::get_producer(CJS_Runtime* pRuntime,
                            CJS_Value* vp,
                            WideString* sError) {
  return getPropertyInternal(pRuntime, vp, "Producer", sError);
}

bool Document::set_producer(CJS_Runtime* pRuntime,
                            const CJS_Value& vp,
                            WideString* sError) {
  return setPropertyInternal(pRuntime, vp, "Producer", sError);
}

bool Document::get_subject(CJS_Runtime* pRuntime,
                           CJS_Value* vp,
                           WideString* sError) {
  return getPropertyInternal(pRuntime, vp, "Subject", sError);
}

bool Document::set_subject(CJS_Runtime* pRuntime,
                           const CJS_Value& vp,
                           WideString* sError) {
  return setPropertyInternal(pRuntime, vp, "Subject", sError);
}

bool Document::get_title(CJS_Runtime* pRuntime,
                         CJS_Value* vp,
                         WideString* sError) {
  if (!m_pFormFillEnv || !m_pFormFillEnv->GetUnderlyingDocument()) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }
  return getPropertyInternal(pRuntime, vp, "Title", sError);
}

bool Document::set_title(CJS_Runtime* pRuntime,
                         const CJS_Value& vp,
                         WideString* sError) {
  if (!m_pFormFillEnv || !m_pFormFillEnv->GetUnderlyingDocument()) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }
  return setPropertyInternal(pRuntime, vp, "Title", sError);
}

bool Document::get_num_pages(CJS_Runtime* pRuntime,
                             CJS_Value* vp,
                             WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }
  vp->Set(pRuntime, m_pFormFillEnv->GetPageCount());
  return true;
}

bool Document::set_num_pages(CJS_Runtime* pRuntime,
                             const CJS_Value& vp,
                             WideString* sError) {
  *sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
  return false;
}

bool Document::get_external(CJS_Runtime* pRuntime,
                            CJS_Value* vp,
                            WideString* sError) {
  // In Chrome case, should always return true.
  vp->Set(pRuntime, true);
  return true;
}

bool Document::set_external(CJS_Runtime* pRuntime,
                            const CJS_Value& vp,
                            WideString* sError) {
  return true;
}

bool Document::get_filesize(CJS_Runtime* pRuntime,
                            CJS_Value* vp,
                            WideString* sError) {
  vp->Set(pRuntime, 0);
  return true;
}

bool Document::set_filesize(CJS_Runtime* pRuntime,
                            const CJS_Value& vp,
                            WideString* sError) {
  *sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
  return false;
}

bool Document::get_mouse_x(CJS_Runtime* pRuntime,
                           CJS_Value* vp,
                           WideString* sError) {
  return true;
}

bool Document::set_mouse_x(CJS_Runtime* pRuntime,
                           const CJS_Value& vp,
                           WideString* sError) {
  return true;
}

bool Document::get_mouse_y(CJS_Runtime* pRuntime,
                           CJS_Value* vp,
                           WideString* sError) {
  return true;
}

bool Document::set_mouse_y(CJS_Runtime* pRuntime,
                           const CJS_Value& vp,
                           WideString* sError) {
  return true;
}

bool Document::get_URL(CJS_Runtime* pRuntime,
                       CJS_Value* vp,
                       WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }
  vp->Set(pRuntime, m_pFormFillEnv->JS_docGetFilePath());
  return true;
}

bool Document::set_URL(CJS_Runtime* pRuntime,
                       const CJS_Value& vp,
                       WideString* sError) {
  *sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
  return false;
}

bool Document::get_base_URL(CJS_Runtime* pRuntime,
                            CJS_Value* vp,
                            WideString* sError) {
  vp->Set(pRuntime, m_cwBaseURL);
  return true;
}

bool Document::set_base_URL(CJS_Runtime* pRuntime,
                            const CJS_Value& vp,
                            WideString* sError) {
  m_cwBaseURL = vp.ToWideString(pRuntime);
  return true;
}

bool Document::get_calculate(CJS_Runtime* pRuntime,
                             CJS_Value* vp,
                             WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  vp->Set(pRuntime, !!pInterForm->IsCalculateEnabled());
  return true;
}

bool Document::set_calculate(CJS_Runtime* pRuntime,
                             const CJS_Value& vp,
                             WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  pInterForm->EnableCalculate(vp.ToBool(pRuntime));
  return true;
}

bool Document::get_document_file_name(CJS_Runtime* pRuntime,
                                      CJS_Value* vp,
                                      WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  WideString wsFilePath = m_pFormFillEnv->JS_docGetFilePath();
  size_t i = wsFilePath.GetLength();
  for (; i > 0; i--) {
    if (wsFilePath[i - 1] == L'\\' || wsFilePath[i - 1] == L'/')
      break;
  }

  if (i > 0 && i < wsFilePath.GetLength())
    vp->Set(pRuntime, wsFilePath.GetBuffer(wsFilePath.GetLength()) + i);
  else
    vp->Set(pRuntime, L"");

  return true;
}

bool Document::set_document_file_name(CJS_Runtime* pRuntime,
                                      const CJS_Value& vp,
                                      WideString* sError) {
  *sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
  return false;
}

bool Document::get_path(CJS_Runtime* pRuntime,
                        CJS_Value* vp,
                        WideString* sError) {
  if (!m_pFormFillEnv) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }
  vp->Set(pRuntime, app::SysPathToPDFPath(m_pFormFillEnv->JS_docGetFilePath()));
  return true;
}

bool Document::set_path(CJS_Runtime* pRuntime,
                        const CJS_Value& vp,
                        WideString* sError) {
  *sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
  return false;
}

bool Document::get_page_window_rect(CJS_Runtime* pRuntime,
                                    CJS_Value* vp,
                                    WideString* sError) {
  return true;
}

bool Document::set_page_window_rect(CJS_Runtime* pRuntime,
                                    const CJS_Value& vp,
                                    WideString* sError) {
  return true;
}

bool Document::get_layout(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError) {
  return true;
}

bool Document::set_layout(CJS_Runtime* pRuntime,
                          const CJS_Value& vp,
                          WideString* sError) {
  return true;
}

pdfium::Optional<CJS_Value> Document::addLink(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::closeDoc(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::getPageBox(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::getAnnot(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  if (params.size() != 2) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return pdfium::Optional<CJS_Value>();
  }
  if (!m_pFormFillEnv) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return pdfium::Optional<CJS_Value>();
  }

  int nPageNo = params[0].ToInt(pRuntime);
  WideString swAnnotName = params[1].ToWideString(pRuntime);
  CPDFSDK_PageView* pPageView = m_pFormFillEnv->GetPageView(nPageNo);
  if (!pPageView)
    return pdfium::Optional<CJS_Value>();

  CPDFSDK_AnnotIteration annotIteration(pPageView, false);
  CPDFSDK_BAAnnot* pSDKBAAnnot = nullptr;
  for (const auto& pSDKAnnotCur : annotIteration) {
    CPDFSDK_BAAnnot* pBAAnnot =
        static_cast<CPDFSDK_BAAnnot*>(pSDKAnnotCur.Get());
    if (pBAAnnot && pBAAnnot->GetAnnotName() == swAnnotName) {
      pSDKBAAnnot = pBAAnnot;
      break;
    }
  }
  if (!pSDKBAAnnot)
    return pdfium::Optional<CJS_Value>();

  v8::Local<v8::Object> pObj =
      pRuntime->NewFxDynamicObj(CJS_Annot::g_nObjDefnID);
  if (pObj.IsEmpty())
    return pdfium::Optional<CJS_Value>();

  CJS_Annot* pJS_Annot =
      static_cast<CJS_Annot*>(pRuntime->GetObjectPrivate(pObj));
  Annot* pAnnot = static_cast<Annot*>(pJS_Annot->GetEmbedObject());
  pAnnot->SetSDKAnnot(pSDKBAAnnot);
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime, pJS_Annot));
}

pdfium::Optional<CJS_Value> Document::getAnnots(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  if (!m_pFormFillEnv) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return pdfium::Optional<CJS_Value>();
  }
  // TODO(tonikitoo): Add support supported parameters as per
  // the PDF spec.

  int nPageNo = m_pFormFillEnv->GetPageCount();
  CJS_Array annots;

  for (int i = 0; i < nPageNo; ++i) {
    CPDFSDK_PageView* pPageView = m_pFormFillEnv->GetPageView(i);
    if (!pPageView)
      return pdfium::Optional<CJS_Value>();

    CPDFSDK_AnnotIteration annotIteration(pPageView, false);
    for (const auto& pSDKAnnotCur : annotIteration) {
      if (!pSDKAnnotCur) {
        sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
        return pdfium::Optional<CJS_Value>();
      }
      v8::Local<v8::Object> pObj =
          pRuntime->NewFxDynamicObj(CJS_Annot::g_nObjDefnID);
      if (pObj.IsEmpty())
        return pdfium::Optional<CJS_Value>();

      CJS_Annot* pJS_Annot =
          static_cast<CJS_Annot*>(pRuntime->GetObjectPrivate(pObj));
      Annot* pAnnot = static_cast<Annot*>(pJS_Annot->GetEmbedObject());
      pAnnot->SetSDKAnnot(static_cast<CPDFSDK_BAAnnot*>(pSDKAnnotCur.Get()));
      annots.SetElement(pRuntime, i, CJS_Value(pRuntime, pJS_Annot));
    }
  }
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime, annots));
}

pdfium::Optional<CJS_Value> Document::getAnnot3D(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  CJS_Value vRet(pRuntime);
  vRet.SetNull(pRuntime);
  return pdfium::Optional<CJS_Value>(vRet);
}

pdfium::Optional<CJS_Value> Document::getAnnots3D(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::getOCGs(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::getLinks(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

bool Document::IsEnclosedInRect(CFX_FloatRect rect, CFX_FloatRect LinkRect) {
  return (rect.left <= LinkRect.left && rect.top <= LinkRect.top &&
          rect.right >= LinkRect.right && rect.bottom >= LinkRect.bottom);
}

pdfium::Optional<CJS_Value> Document::addIcon(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  if (params.size() != 2) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return pdfium::Optional<CJS_Value>();
  }

  WideString swIconName = params[0].ToWideString(pRuntime);
  if (params[1].GetType() != CJS_Value::VT_object) {
    sError = JSGetStringFromID(IDS_STRING_JSTYPEERROR);
    return pdfium::Optional<CJS_Value>();
  }

  v8::Local<v8::Object> pJSIcon = params[1].ToV8Object(pRuntime);
  if (CFXJS_Engine::GetObjDefnID(pJSIcon) != CJS_Icon::g_nObjDefnID) {
    sError = JSGetStringFromID(IDS_STRING_JSTYPEERROR);
    return pdfium::Optional<CJS_Value>();
  }

  if (!params[1].ToObject(pRuntime)->GetEmbedObject()) {
    sError = JSGetStringFromID(IDS_STRING_JSTYPEERROR);
    return pdfium::Optional<CJS_Value>();
  }

  m_IconNames.push_back(swIconName);
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

bool Document::get_icons(CJS_Runtime* pRuntime,
                         CJS_Value* vp,
                         WideString* sError) {
  if (m_IconNames.empty()) {
    vp->SetNull(pRuntime);
    return true;
  }

  CJS_Array Icons;
  int i = 0;
  for (const auto& name : m_IconNames) {
    v8::Local<v8::Object> pObj =
        pRuntime->NewFxDynamicObj(CJS_Icon::g_nObjDefnID);
    if (pObj.IsEmpty())
      return false;

    CJS_Icon* pJS_Icon =
        static_cast<CJS_Icon*>(pRuntime->GetObjectPrivate(pObj));
    Icon* pIcon = static_cast<Icon*>(pJS_Icon->GetEmbedObject());
    pIcon->SetIconName(name);
    Icons.SetElement(pRuntime, i++, CJS_Value(pRuntime, pJS_Icon));
  }

  vp->Set(pRuntime, Icons);
  return true;
}

bool Document::set_icons(CJS_Runtime* pRuntime,
                         const CJS_Value& vp,
                         WideString* sError) {
  *sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
  return false;
}

pdfium::Optional<CJS_Value> Document::getIcon(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  if (params.size() != 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return pdfium::Optional<CJS_Value>();
  }

  WideString swIconName = params[0].ToWideString(pRuntime);
  auto it = std::find(m_IconNames.begin(), m_IconNames.end(), swIconName);
  if (it == m_IconNames.end())
    return pdfium::Optional<CJS_Value>();

  v8::Local<v8::Object> pObj =
      pRuntime->NewFxDynamicObj(CJS_Icon::g_nObjDefnID);
  if (pObj.IsEmpty())
    return pdfium::Optional<CJS_Value>();

  CJS_Icon* pJS_Icon = static_cast<CJS_Icon*>(pRuntime->GetObjectPrivate(pObj));
  Icon* pIcon = static_cast<Icon*>(pJS_Icon->GetEmbedObject());
  pIcon->SetIconName(*it);
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime, pJS_Icon));
}

pdfium::Optional<CJS_Value> Document::removeIcon(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Unsafe, no supported.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::createDataObject(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Unsafe, not implemented.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

bool Document::get_media(CJS_Runtime* pRuntime,
                         CJS_Value* vp,
                         WideString* sError) {
  return true;
}

bool Document::set_media(CJS_Runtime* pRuntime,
                         const CJS_Value& vp,
                         WideString* sError) {
  return true;
}

pdfium::Optional<CJS_Value> Document::calculateNow(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  if (!m_pFormFillEnv) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return pdfium::Optional<CJS_Value>();
  }
  if (!(m_pFormFillEnv->GetPermissions(FPDFPERM_MODIFY) ||
        m_pFormFillEnv->GetPermissions(FPDFPERM_ANNOT_FORM) ||
        m_pFormFillEnv->GetPermissions(FPDFPERM_FILL_FORM))) {
    sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
    return pdfium::Optional<CJS_Value>();
  }

  m_pFormFillEnv->GetInterForm()->OnCalculate();
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

bool Document::get_collab(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError) {
  return true;
}

bool Document::set_collab(CJS_Runtime* pRuntime,
                          const CJS_Value& vp,
                          WideString* sError) {
  return true;
}

pdfium::Optional<CJS_Value> Document::getPageNthWord(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  if (!m_pFormFillEnv) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return pdfium::Optional<CJS_Value>();
  }
  if (!m_pFormFillEnv->GetPermissions(FPDFPERM_EXTRACT_ACCESS)) {
    sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
    return pdfium::Optional<CJS_Value>();
  }

  // TODO(tsepez): check maximum allowable params.

  int nPageNo = params.size() > 0 ? params[0].ToInt(pRuntime) : 0;
  int nWordNo = params.size() > 1 ? params[1].ToInt(pRuntime) : 0;
  bool bStrip = params.size() > 2 ? params[2].ToBool(pRuntime) : true;

  CPDF_Document* pDocument = m_pFormFillEnv->GetPDFDocument();
  if (!pDocument)
    return pdfium::Optional<CJS_Value>();

  if (nPageNo < 0 || nPageNo >= pDocument->GetPageCount()) {
    sError = JSGetStringFromID(IDS_STRING_JSVALUEERROR);
    return pdfium::Optional<CJS_Value>();
  }

  CPDF_Dictionary* pPageDict = pDocument->GetPage(nPageNo);
  if (!pPageDict)
    return pdfium::Optional<CJS_Value>();

  CPDF_Page page(pDocument, pPageDict, true);
  page.ParseContent();

  int nWords = 0;
  WideString swRet;
  for (auto& pPageObj : *page.GetPageObjectList()) {
    if (pPageObj->IsText()) {
      CPDF_TextObject* pTextObj = pPageObj->AsText();
      int nObjWords = CountWords(pTextObj);
      if (nWords + nObjWords >= nWordNo) {
        swRet = GetObjWordStr(pTextObj, nWordNo - nWords);
        break;
      }
      nWords += nObjWords;
    }
  }

  if (bStrip) {
    swRet.TrimLeft();
    swRet.TrimRight();
  }

  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime, swRet.c_str()));
}

pdfium::Optional<CJS_Value> Document::getPageNthWordQuads(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  if (!m_pFormFillEnv) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return pdfium::Optional<CJS_Value>();
  }
  if (!m_pFormFillEnv->GetPermissions(FPDFPERM_EXTRACT_ACCESS)) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return pdfium::Optional<CJS_Value>();
  }

  // Not supported?
  return pdfium::Optional<CJS_Value>();
}

pdfium::Optional<CJS_Value> Document::getPageNumWords(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  if (!m_pFormFillEnv) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return pdfium::Optional<CJS_Value>();
  }
  if (!m_pFormFillEnv->GetPermissions(FPDFPERM_EXTRACT_ACCESS)) {
    sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
    return pdfium::Optional<CJS_Value>();
  }

  int nPageNo = params.size() > 0 ? params[0].ToInt(pRuntime) : 0;
  CPDF_Document* pDocument = m_pFormFillEnv->GetPDFDocument();
  if (nPageNo < 0 || nPageNo >= pDocument->GetPageCount()) {
    sError = JSGetStringFromID(IDS_STRING_JSVALUEERROR);
    return pdfium::Optional<CJS_Value>();
  }

  CPDF_Dictionary* pPageDict = pDocument->GetPage(nPageNo);
  if (!pPageDict)
    return pdfium::Optional<CJS_Value>();

  CPDF_Page page(pDocument, pPageDict, true);
  page.ParseContent();

  int nWords = 0;
  for (auto& pPageObj : *page.GetPageObjectList()) {
    if (pPageObj->IsText())
      nWords += CountWords(pPageObj->AsText());
  }

  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime, nWords));
}

pdfium::Optional<CJS_Value> Document::getPrintParams(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  v8::Local<v8::Object> pRetObj =
      pRuntime->NewFxDynamicObj(CJS_PrintParamsObj::g_nObjDefnID);
  if (pRetObj.IsEmpty())
    return pdfium::Optional<CJS_Value>();

  // Not implemented yet.

  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime, pRetObj));
}

#define ISLATINWORD(u) (u != 0x20 && u <= 0x28FF)

int Document::CountWords(CPDF_TextObject* pTextObj) {
  if (!pTextObj)
    return 0;

  int nWords = 0;

  CPDF_Font* pFont = pTextObj->GetFont();
  if (!pFont)
    return 0;

  bool bIsLatin = false;

  for (int i = 0, sz = pTextObj->CountChars(); i < sz; i++) {
    uint32_t charcode = CPDF_Font::kInvalidCharCode;
    float kerning;

    pTextObj->GetCharInfo(i, &charcode, &kerning);
    WideString swUnicode = pFont->UnicodeFromCharCode(charcode);

    uint16_t unicode = 0;
    if (swUnicode.GetLength() > 0)
      unicode = swUnicode[0];

    if (ISLATINWORD(unicode) && bIsLatin)
      continue;

    bIsLatin = ISLATINWORD(unicode);
    if (unicode != 0x20)
      nWords++;
  }

  return nWords;
}

WideString Document::GetObjWordStr(CPDF_TextObject* pTextObj, int nWordIndex) {
  WideString swRet;

  CPDF_Font* pFont = pTextObj->GetFont();
  if (!pFont)
    return L"";

  int nWords = 0;
  bool bIsLatin = false;

  for (int i = 0, sz = pTextObj->CountChars(); i < sz; i++) {
    uint32_t charcode = CPDF_Font::kInvalidCharCode;
    float kerning;

    pTextObj->GetCharInfo(i, &charcode, &kerning);
    WideString swUnicode = pFont->UnicodeFromCharCode(charcode);

    uint16_t unicode = 0;
    if (swUnicode.GetLength() > 0)
      unicode = swUnicode[0];

    if (ISLATINWORD(unicode) && bIsLatin) {
    } else {
      bIsLatin = ISLATINWORD(unicode);
      if (unicode != 0x20)
        nWords++;
    }

    if (nWords - 1 == nWordIndex)
      swRet += unicode;
  }

  return swRet;
}

bool Document::get_zoom(CJS_Runtime* pRuntime,
                        CJS_Value* vp,
                        WideString* sError) {
  return true;
}

bool Document::set_zoom(CJS_Runtime* pRuntime,
                        const CJS_Value& vp,
                        WideString* sError) {
  return true;
}

bool Document::get_zoom_type(CJS_Runtime* pRuntime,
                             CJS_Value* vp,
                             WideString* sError) {
  return true;
}
bool Document::set_zoom_type(CJS_Runtime* pRuntime,
                             const CJS_Value& vp,
                             WideString* sError) {
  return true;
}

pdfium::Optional<CJS_Value> Document::deletePages(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Unsafe, no supported.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::extractPages(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Unsafe, not supported.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::insertPages(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Unsafe, not supported.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::replacePages(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Unsafe, not supported.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::getURL(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  // Unsafe, not supported.
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

pdfium::Optional<CJS_Value> Document::gotoNamedDest(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& params,
    WideString& sError) {
  if (params.size() != 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return pdfium::Optional<CJS_Value>();
  }
  if (!m_pFormFillEnv) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return pdfium::Optional<CJS_Value>();
  }

  WideString wideName = params[0].ToWideString(pRuntime);
  CPDF_Document* pDocument = m_pFormFillEnv->GetPDFDocument();
  if (!pDocument)
    return pdfium::Optional<CJS_Value>();

  CPDF_NameTree nameTree(pDocument, "Dests");
  CPDF_Array* destArray = nameTree.LookupNamedDest(pDocument, wideName);
  if (!destArray)
    return pdfium::Optional<CJS_Value>();

  CPDF_Dest dest(destArray);
  const CPDF_Array* arrayObject = ToArray(dest.GetObject());
  std::vector<float> scrollPositionArray;
  if (arrayObject) {
    for (size_t i = 2; i < arrayObject->GetCount(); i++)
      scrollPositionArray.push_back(arrayObject->GetFloatAt(i));
  }
  pRuntime->BeginBlock();
  m_pFormFillEnv->DoGoToAction(dest.GetPageIndex(pDocument), dest.GetZoomMode(),
                               scrollPositionArray.data(),
                               scrollPositionArray.size());
  pRuntime->EndBlock();
  return pdfium::Optional<CJS_Value>(CJS_Value(pRuntime));
}

void Document::AddDelayData(CJS_DelayData* pData) {
  m_DelayData.push_back(std::unique_ptr<CJS_DelayData>(pData));
}

void Document::DoFieldDelay(const WideString& sFieldName, int nControlIndex) {
  std::vector<std::unique_ptr<CJS_DelayData>> delayed_data;
  auto iter = m_DelayData.begin();
  while (iter != m_DelayData.end()) {
    auto old = iter++;
    if ((*old)->sFieldName == sFieldName &&
        (*old)->nControlIndex == nControlIndex) {
      delayed_data.push_back(std::move(*old));
      m_DelayData.erase(old);
    }
  }

  for (const auto& pData : delayed_data)
    Field::DoDelay(m_pFormFillEnv.Get(), pData.get());
}

CJS_Document* Document::GetCJSDoc() const {
  return static_cast<CJS_Document*>(m_pJSObject.Get());
}
