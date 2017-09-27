// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_FX_FONT_H_
#define CORE_FXGE_FX_FONT_H_

#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/fx_dib.h"
#include "core/fxge/fx_freetype.h"

class CFX_FaceCache;
class CFX_GlyphBitmap;
class CFX_PathData;
class IFX_SeekableReadStream;

#if defined _SKIA_SUPPORT_ || defined _SKIA_SUPPORT_PATHS_
class SkTypeface;

using CFX_TypeFace = SkTypeface;
#endif

/* Font pitch and family flags */
#define FXFONT_FF_FIXEDPITCH 1
#define FXFONT_FF_ROMAN (1 << 4)
#define FXFONT_FF_SCRIPT (4 << 4)

/* Typical weight values */
#define FXFONT_FW_NORMAL 400
#define FXFONT_FW_BOLD 700

/* Font styles as defined in PDF 1.7 Table 5.20 */
#define FXFONT_FIXED_PITCH (1 << 0)
#define FXFONT_SERIF (1 << 1)
#define FXFONT_SYMBOLIC (1 << 2)
#define FXFONT_SCRIPT (1 << 3)
#define FXFONT_NONSYMBOLIC (1 << 5)
#define FXFONT_ITALIC (1 << 6)
#define FXFONT_ALLCAP (1 << 16)
#define FXFONT_SMALLCAP (1 << 17)
#define FXFONT_BOLD (1 << 18)

/* Other font flags */
#define FXFONT_USEEXTERNATTR 0x80000
#define FXFONT_CIDFONT 0x100000
#ifdef PDF_ENABLE_XFA
#define FXFONT_EXACTMATCH 0x80000000
#endif  // PDF_ENABLE_XFA

#define CHARSET_FLAG_ANSI 1
#define CHARSET_FLAG_SYMBOL 2
#define CHARSET_FLAG_SHIFTJIS 4
#define CHARSET_FLAG_BIG5 8
#define CHARSET_FLAG_GB 16
#define CHARSET_FLAG_KOREAN 32

#define GET_TT_SHORT(w) (uint16_t)(((w)[0] << 8) | (w)[1])
#define GET_TT_LONG(w) \
  (uint32_t)(((w)[0] << 24) | ((w)[1] << 16) | ((w)[2] << 8) | (w)[3])

// Sets the given transform on the font, and resets it to the identity when it
// goes out of scope.
class ScopedFontTransform {
 public:
  ScopedFontTransform(FT_Face face, FXFT_Matrix* matrix);
  ~ScopedFontTransform();

 private:
  FT_Face m_Face;
};

class CFX_GlyphBitmap {
 public:
  CFX_GlyphBitmap();
  ~CFX_GlyphBitmap();

  int m_Top;
  int m_Left;
  RetainPtr<CFX_DIBitmap> m_pBitmap;
};

inline CFX_GlyphBitmap::CFX_GlyphBitmap()
    : m_pBitmap(pdfium::MakeRetain<CFX_DIBitmap>()) {}

inline CFX_GlyphBitmap::~CFX_GlyphBitmap() {}

class FXTEXT_GLYPHPOS {
 public:
  FXTEXT_GLYPHPOS();
  FXTEXT_GLYPHPOS(const FXTEXT_GLYPHPOS&);
  ~FXTEXT_GLYPHPOS();

  const CFX_GlyphBitmap* m_pGlyph;
  CFX_Point m_Origin;
  CFX_PointF m_fOrigin;
};

FX_RECT FXGE_GetGlyphsBBox(const std::vector<FXTEXT_GLYPHPOS>& glyphs,
                           int anti_alias,
                           float retinaScaleX,
                           float retinaScaleY);

ByteString GetNameFromTT(const uint8_t* name_table,
                         uint32_t name_table_size,
                         uint32_t name);

int PDF_GetStandardFontName(ByteString* name);

#endif  // CORE_FXGE_FX_FONT_H_
