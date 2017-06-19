// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_charposlist.h"

#include "core/fpdfapi/font/cpdf_cidfont.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "third_party/base/stl_util.h"

CPDF_CharPosList::CPDF_CharPosList() {
  m_pCharPos = nullptr;
  m_nChars = 0;
}

CPDF_CharPosList::~CPDF_CharPosList() {
  FX_Free(m_pCharPos);
}

void CPDF_CharPosList::Load(const std::vector<uint32_t>& charCodes,
                            const std::vector<float>& charPos,
                            CPDF_Font* pFont,
                            float FontSize) {
  int nChars = pdfium::CollectionSize<int>(charCodes);
  m_pCharPos = FX_Alloc(FXTEXT_CHARPOS, nChars);
  m_nChars = 0;
  CPDF_CIDFont* pCIDFont = pFont->AsCIDFont();
  bool bVertWriting = pCIDFont && pCIDFont->IsVertWriting();
  for (int iChar = 0; iChar < nChars; iChar++) {
    uint32_t CharCode = charCodes[iChar];
    if (CharCode == static_cast<uint32_t>(-1))
      continue;

    bool bVert = false;
    FXTEXT_CHARPOS& charpos = m_pCharPos[m_nChars++];
    if (pCIDFont)
      charpos.m_bFontStyle = true;

    charpos.m_GlyphIndex = pFont->GlyphFromCharCode(CharCode, &bVert);
    uint32_t GlyphID = charpos.m_GlyphIndex;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
    charpos.m_ExtGID = pFont->GlyphFromCharCodeExt(CharCode);
    GlyphID = charpos.m_ExtGID;
#endif
    CFX_Font* currentFont;
    if (GlyphID != static_cast<uint32_t>(-1)) {
      charpos.m_FallbackFontPosition = -1;
      currentFont = pFont->GetFont();
    } else {
      charpos.m_FallbackFontPosition =
          pFont->FallbackFontFromCharcode(CharCode);
      charpos.m_GlyphIndex = pFont->FallbackGlyphFromCharcode(
          charpos.m_FallbackFontPosition, CharCode);
      currentFont = pFont->GetFontFallback(charpos.m_FallbackFontPosition);
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
      charpos.m_ExtGID = charpos.m_GlyphIndex;
#endif
    }

    int PDFWidth = pFont->IsEmbedded() ? 0 : pFont->GetCharWidthF(CharCode);
    if (!pFont->IsEmbedded() && !pFont->IsCIDFont())
      charpos.m_FontCharWidth = PDFWidth;
    else
      charpos.m_FontCharWidth = 0;

    charpos.m_Origin = CFX_PointF(iChar ? charPos[iChar - 1] : 0, 0);
    charpos.m_bGlyphAdjust = false;

    float scalingFactor = 1.0f;
    if (!pFont->IsEmbedded() && !bVertWriting && PDFWidth &&
        !(currentFont->GetSubstFont()->m_SubstFlags & FXFONT_SUBST_MM)) {
      int FTWidth =
          currentFont ? currentFont->GetGlyphWidth(charpos.m_GlyphIndex) : 0;
      if (FTWidth && PDFWidth > FTWidth + 1) {
        charpos.m_Origin.x += (PDFWidth - FTWidth) * FontSize / 2000.0f;
      } else if (FTWidth && PDFWidth < FTWidth) {
        scalingFactor = static_cast<float>(PDFWidth) / FTWidth;
        ASSERT(scalingFactor >= 0.0f);
        charpos.m_AdjustMatrix[1] = 0.0f;
        charpos.m_AdjustMatrix[2] = 0.0f;
        charpos.m_AdjustMatrix[3] = 1.0f;
        charpos.m_bGlyphAdjust = true;
        charpos.m_AdjustMatrix[0] = scalingFactor;
      }
    }
    if (!pCIDFont)
      continue;

    uint16_t CID = pCIDFont->CIDFromCharCode(CharCode);
    if (bVertWriting) {
      charpos.m_Origin = CFX_PointF(0, charpos.m_Origin.x);

      short vx;
      short vy;
      pCIDFont->GetVertOrigin(CID, vx, vy);
      charpos.m_Origin.x -= FontSize * vx / 1000;
      charpos.m_Origin.y -= FontSize * vy / 1000;
    }

    const uint8_t* pTransform = pCIDFont->GetCIDTransform(CID);
    if (pTransform && !bVert) {
      charpos.m_AdjustMatrix[0] =
          pCIDFont->CIDTransformToFloat(pTransform[0]) * scalingFactor;
      charpos.m_AdjustMatrix[1] =
          pCIDFont->CIDTransformToFloat(pTransform[1]) * scalingFactor;
      charpos.m_AdjustMatrix[2] = pCIDFont->CIDTransformToFloat(pTransform[2]);
      charpos.m_AdjustMatrix[3] = pCIDFont->CIDTransformToFloat(pTransform[3]);
      charpos.m_Origin.x +=
          pCIDFont->CIDTransformToFloat(pTransform[4]) * FontSize;
      charpos.m_Origin.y +=
          pCIDFont->CIDTransformToFloat(pTransform[5]) * FontSize;
      charpos.m_bGlyphAdjust = true;
    }
  }
}
