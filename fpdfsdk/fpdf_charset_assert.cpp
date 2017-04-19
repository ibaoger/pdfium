// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_codepage.h"
#include "public/fpdf_sysfontinfo.h"

static_assert(FXFONT_ANSI_CHARSET == FX_CHARSET_ANSI, "Charset must match");
static_assert(FXFONT_DEFAULT_CHARSET == FX_CHARSET_Default,
              "Charset must match");
static_assert(FXFONT_SYMBOL_CHARSET == FX_CHARSET_Symbol, "Charset must match");
static_assert(FXFONT_SHIFTJIS_CHARSET == FX_CHARSET_ShiftJIS,
              "Charset must match");
static_assert(FXFONT_HANGEUL_CHARSET == FX_CHARSET_Hangul,
              "Charset must match");
static_assert(FXFONT_GB2312_CHARSET == FX_CHARSET_ChineseSimplified,
              "Charset must match");
static_assert(FXFONT_CHINESEBIG5_CHARSET == FX_CHARSET_ChineseTraditional,
              "Charset must match");
