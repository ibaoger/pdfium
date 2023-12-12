// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_FREETYPE_FX_FREETYPE_H_
#define CORE_FXGE_FREETYPE_FX_FREETYPE_H_

#include <ft2build.h>

#include <memory>

#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_LCD_FILTER_H
#include FT_MULTIPLE_MASTERS_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_TABLES_H

using FXFT_LibraryRec = struct FT_LibraryRec_;
using FXFT_FaceRec = struct FT_FaceRec_;
using FXFT_StreamRec = struct FT_StreamRec_;

struct FXFTFaceRecDeleter {
  inline void operator()(FXFT_FaceRec* pRec) { FT_Done_Face(pRec); }
};

struct FXFTLibraryRecDeleter {
  inline void operator()(FXFT_LibraryRec* pRec) { FT_Done_FreeType(pRec); }
};

struct FXFTMMVarDeleter {
  void operator()(FT_MM_Var* variation_desc);
};

using ScopedFXFTFaceRec = std::unique_ptr<FXFT_FaceRec, FXFTFaceRecDeleter>;
using ScopedFXFTLibraryRec =
    std::unique_ptr<FXFT_LibraryRec, FXFTLibraryRecDeleter>;

class ScopedFXFTMMVar {
 public:
  explicit ScopedFXFTMMVar(FXFT_FaceRec* face);
  ~ScopedFXFTMMVar();

  explicit operator bool() const { return !!variation_desc_; }

  FT_Pos GetAxisDefault(size_t index) const;
  FT_Long GetAxisMin(size_t index) const;
  FT_Long GetAxisMax(size_t index) const;

 private:
  std::unique_ptr<FT_MM_Var, FXFTMMVarDeleter> const variation_desc_;
};

#define FXFT_Select_Charmap(face, encoding) \
  FT_Select_Charmap(face, static_cast<FT_Encoding>(encoding))
#define FXFT_Render_Glyph(face, mode) \
  FT_Render_Glyph((face)->glyph, static_cast<enum FT_Render_Mode_>(mode))

#define FXFT_Get_Glyph_HoriBearingX(face) (face)->glyph->metrics.horiBearingX
#define FXFT_Get_Glyph_HoriBearingY(face) (face)->glyph->metrics.horiBearingY
#define FXFT_Get_Glyph_Width(face) (face)->glyph->metrics.width
#define FXFT_Get_Glyph_Height(face) (face)->glyph->metrics.height
#define FXFT_Get_Charmap_Encoding(charmap) (charmap)->encoding
#define FXFT_Get_Charmap_PlatformID(charmap) (charmap)->platform_id
#define FXFT_Get_Charmap_EncodingID(charmap) (charmap)->encoding_id
#define FXFT_Get_Face_UnitsPerEM(face) (face)->units_per_EM
#define FXFT_Get_Face_Height(face) (face)->height
#define FXFT_Get_Face_Ascender(face) (face)->ascender
#define FXFT_Get_Face_Descender(face) (face)->descender
#define FXFT_Get_Glyph_HoriAdvance(face) (face)->glyph->metrics.horiAdvance
#define FXFT_Get_Glyph_Outline(face) &((face)->glyph->outline)
#define FXFT_Get_Glyph_Bitmap(face) (face)->glyph->bitmap
#define FXFT_Get_Bitmap_Width(bitmap) (bitmap).width
#define FXFT_Get_Bitmap_Rows(bitmap) (bitmap).rows
#define FXFT_Get_Bitmap_PixelMode(bitmap) (bitmap).pixel_mode
#define FXFT_Get_Bitmap_Pitch(bitmap) (bitmap).pitch
#define FXFT_Get_Bitmap_Buffer(bitmap) (bitmap).buffer
#define FXFT_Get_Glyph_BitmapLeft(face) (face)->glyph->bitmap_left
#define FXFT_Get_Glyph_BitmapTop(face) (face)->glyph->bitmap_top

int FXFT_unicode_from_adobe_name(const char* glyph_name);
void FXFT_adobe_name_from_unicode(char* name, wchar_t unicode);

#endif  // CORE_FXGE_FREETYPE_FX_FREETYPE_H_
