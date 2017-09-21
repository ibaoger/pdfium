// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FONTMGR_H_
#define XFA_FXFA_CXFA_FONTMGR_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fgas/font/cfgas_pdffontmgr.h"
#include "xfa/fxfa/cxfa_deffontmgr.h"
#include "xfa/fxfa/fxfa.h"

class CPDF_Font;

struct XFA_FONTINFO {
  uint32_t dwFontNameHash;
  const wchar_t* pPsName;
  const wchar_t* pReplaceFont;
  uint16_t dwStyles;
  uint16_t wCodePage;
};

WideString XFA_LocalFontNameToEnglishName(const WideStringView& wsLocalName);
const XFA_FONTINFO* XFA_GetFontINFOByFontName(const WideStringView& wsFontName);

class CXFA_FontMgr {
 public:
  CXFA_FontMgr();
  ~CXFA_FontMgr();

  RetainPtr<CFGAS_GEFont> GetFont(CXFA_FFDoc* hDoc,
                                  const WideStringView& wsFontFamily,
                                  uint32_t dwFontStyles,
                                  uint16_t wCodePage = 0xFFFF);
  void LoadDocFonts(CXFA_FFDoc* hDoc);
  void ReleaseDocFonts(CXFA_FFDoc* hDoc);
  void SetDefFontMgr(std::unique_ptr<CXFA_DefFontMgr> pFontMgr);

 private:
  std::unique_ptr<CXFA_DefFontMgr> m_pDefFontMgr;
  std::map<CXFA_FFDoc*, std::unique_ptr<CFGAS_PDFFontMgr>> m_PDFFontMgrMap;
  std::map<ByteString, RetainPtr<CFGAS_GEFont>> m_FontMap;
};

#endif  //  XFA_FXFA_CXFA_FONTMGR_H_
