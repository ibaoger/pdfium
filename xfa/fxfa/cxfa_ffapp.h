// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFAPP_H_
#define XFA_FXFA_CXFA_FFAPP_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/retain_ptr.h"
#include "xfa/fgas/font/cfgas_defaultfontmanager.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fxfa/cxfa_fwladapterwidgetmgr.h"
#include "xfa/fxfa/cxfa_fwltheme.h"
#include "xfa/fxfa/fxfa.h"

class CFWL_WidgetMgr;
class CXFA_FFDocHandler;
class IFWL_AdapterTimerMgr;

class CXFA_FFApp {
 public:
  explicit CXFA_FFApp(IXFA_AppProvider* pProvider);
  ~CXFA_FFApp();

  std::unique_ptr<CXFA_FFDoc> CreateDoc(IXFA_DocEnvironment* pDocEnvironment,
                                        CPDF_Document* pPDFDoc);

  CXFA_FFDocHandler* GetDocHandler();

  CFWL_WidgetMgr* GetFWLWidgetMgr() { return m_FWLApp.GetWidgetMgr(); }
  CFGAS_FontMgr* GetFDEFontMgr();
  const CXFA_FWLTheme* GetFWLTheme() const { return &m_FWLTheme; }

  IXFA_AppProvider* GetAppProvider() const { return m_pProvider.Get(); }
  CFWL_App* GetFWLApp() { return &m_FWLApp; }
  IFWL_AdapterTimerMgr* GetTimerMgr() const {
    return m_pProvider->GetTimerMgr();
  }

  RetainPtr<CFGAS_GEFont> GetFont(CFGAS_PDFFontMgr* pMgr,
                                  const WideStringView& wsFontFamily,
                                  uint32_t dwFontStyles);

  void ClearEventTargets();

 private:
  std::unique_ptr<CXFA_FFDocHandler> m_pDocHandler;
  CFX_UnownedPtr<IXFA_AppProvider> const m_pProvider;

  // The fonts stored in the font manager may have been created by the default
  // font manager. The GEFont::LoadFont call takes the manager as a param and
  // stores it internally. When you destroy the GEFont it tries to unregister
  // from the font manager and if the default font manager was destroyed first
  // you get a use-after-free. The m_FWLTheme can try to cleanup a GEFont
  // when it frees, so make sure it gets cleaned up first. That requires
  // m_FWLApp to be cleaned up as well.
  //
  // TODO(dsinclair): The GEFont should have the FontMgr as the pointer instead
  // of the DEFFontMgr so this goes away. Bug 561.
  std::unique_ptr<CFGAS_FontMgr> m_pFDEFontMgr;
  CFGAS_DefaultFontManager m_DefFontMgr;
  std::map<ByteString, RetainPtr<CFGAS_GEFont>> m_FontCache;

#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
  std::unique_ptr<CFX_FontSourceEnum_File> m_pFontSource;
#endif

  CXFA_FWLAdapterWidgetMgr m_AdapterWidgetMgr;

  // |m_FWLApp| has to be released first, then |m_FWLTheme| since the former
  // may refers to theme manager and the latter refers to font manager.
  CXFA_FWLTheme m_FWLTheme;
  CFWL_App m_FWLApp;
};

#endif  // XFA_FXFA_CXFA_FFAPP_H_
