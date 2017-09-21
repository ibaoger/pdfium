// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffapp.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fgas/font/fgas_fontutils.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdochandler.h"
#include "xfa/fxfa/cxfa_ffwidgethandler.h"
#include "xfa/fxfa/cxfa_fwladapterwidgetmgr.h"
#include "xfa/fxfa/cxfa_fwltheme.h"

CXFA_FFApp::CXFA_FFApp(IXFA_AppProvider* pProvider)
    : m_pProvider(pProvider),
      m_FWLTheme(this),
      m_FWLApp(this, &m_AdapterWidgetMgr) {}

CXFA_FFApp::~CXFA_FFApp() {}

CXFA_FFDocHandler* CXFA_FFApp::GetDocHandler() {
  if (!m_pDocHandler)
    m_pDocHandler = pdfium::MakeUnique<CXFA_FFDocHandler>();
  return m_pDocHandler.get();
}

std::unique_ptr<CXFA_FFDoc> CXFA_FFApp::CreateDoc(
    IXFA_DocEnvironment* pDocEnvironment,
    CPDF_Document* pPDFDoc) {
  if (!pPDFDoc)
    return nullptr;

  auto pDoc = pdfium::MakeUnique<CXFA_FFDoc>(this, pDocEnvironment);
  if (!pDoc->OpenDoc(pPDFDoc))
    return nullptr;

  return pDoc;
}

CFGAS_FontMgr* CXFA_FFApp::GetFDEFontMgr() {
  if (!m_pFDEFontMgr) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    m_pFDEFontMgr = CFGAS_FontMgr::Create(FX_GetDefFontEnumerator());
#else
    m_pFontSource = pdfium::MakeUnique<CFX_FontSourceEnum_File>();
    m_pFDEFontMgr = CFGAS_FontMgr::Create(m_pFontSource.get());
#endif
  }
  return m_pFDEFontMgr.get();
}

void CXFA_FFApp::ClearEventTargets() {
  m_FWLApp.GetNoteDriver()->ClearEventTargets();
}

CFX_RetainPtr<CFGAS_GEFont> CXFA_FFApp::GetFont(
    CFGAS_PDFFontMgr* pMgr,
    const WideStringView& wsFontFamily,
    uint32_t dwFontStyles) {
  uint32_t dwHash = FX_HashCode_GetW(wsFontFamily, false);
  ByteString bsKey;
  bsKey.Format("%u%u%u", dwHash, dwFontStyles, 0xFFFF);
  auto iter = m_FontCache.find(bsKey);
  if (iter != m_FontCache.end())
    return iter->second;

  WideString wsEnglishName = FGAS_FontNameToEnglishName(wsFontFamily);

  CPDF_Font* pPDFFont = nullptr;
  CFX_RetainPtr<CFGAS_GEFont> pFont;
  if (pMgr) {
    pFont = pMgr->GetFont(wsEnglishName.AsStringView(), dwFontStyles, &pPDFFont,
                          true);
    if (pFont)
      return pFont;
  }
  if (!pFont)
    pFont = m_DefFontMgr.GetFont(GetFDEFontMgr(), wsFontFamily, dwFontStyles);

  if (!pFont && pMgr) {
    pPDFFont = nullptr;
    pFont = pMgr->GetFont(wsEnglishName.AsStringView(), dwFontStyles, &pPDFFont,
                          false);
    if (pFont)
      return pFont;
  }
  if (!pFont) {
    pFont = m_DefFontMgr.GetDefaultFont(GetFDEFontMgr(), wsFontFamily,
                                        dwFontStyles);
  }
  if (!pFont)
    return nullptr;

  if (pPDFFont) {
    pMgr->SetFont(pFont, pPDFFont);
    pFont->SetFontProvider(pMgr);
  }
  m_FontCache[bsKey] = pFont;
  return pFont;
}
