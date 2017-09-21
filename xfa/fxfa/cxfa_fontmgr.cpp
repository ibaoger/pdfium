// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_fontmgr.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/font/fgas_fontutils.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"

CXFA_FontMgr::CXFA_FontMgr() {}

CXFA_FontMgr::~CXFA_FontMgr() {}

RetainPtr<CFGAS_GEFont> CXFA_FontMgr::GetFont(
    CXFA_FFDoc* hDoc,
    const WideStringView& wsFontFamily,
    uint32_t dwFontStyles,
    uint16_t wCodePage) {
  uint32_t dwHash = FX_HashCode_GetW(wsFontFamily, false);
  ByteString bsKey;
  bsKey.Format("%u%u%u", dwHash, dwFontStyles, wCodePage);
  auto iter = m_FontMap.find(bsKey);
  if (iter != m_FontMap.end())
    return iter->second;

  WideString wsEnglishName = FGAS_FontNameToEnglishName(wsFontFamily);

  CFGAS_PDFFontMgr* pMgr = hDoc->GetPDFFontMgr();
  CPDF_Font* pPDFFont = nullptr;
  RetainPtr<CFGAS_GEFont> pFont;
  if (pMgr) {
    pFont = pMgr->GetFont(wsEnglishName.AsStringView(), dwFontStyles, &pPDFFont,
                          true);
    if (pFont)
      return pFont;
  }
  if (!pFont)
    pFont = m_DefFontMgr.GetFont(hDoc->GetApp()->GetFDEFontMgr(), wsFontFamily,
                                 dwFontStyles, wCodePage);

  if (!pFont && pMgr) {
    pPDFFont = nullptr;
    pFont = pMgr->GetFont(wsEnglishName.AsStringView(), dwFontStyles, &pPDFFont,
                          false);
    if (pFont)
      return pFont;
  }
  if (!pFont) {
    pFont = m_DefFontMgr.GetDefaultFont(hDoc->GetApp()->GetFDEFontMgr(),
                                        wsFontFamily, dwFontStyles, wCodePage);
  }

  if (!pFont)
    return nullptr;

  if (pPDFFont) {
    pMgr->SetFont(pFont, pPDFFont);
    pFont->SetFontProvider(pMgr);
  }
  m_FontMap[bsKey] = pFont;
  return pFont;
}
