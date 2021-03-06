// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/scoped_font_transform.h"

namespace {

void ResetTransform(FT_Face face) {
  FXFT_Matrix matrix;
  matrix.xx = 0x10000L;
  matrix.xy = 0;
  matrix.yx = 0;
  matrix.yy = 0x10000L;
  FXFT_Set_Transform(face, &matrix, 0);
}

}  // namespace

ScopedFontTransform::ScopedFontTransform(FT_Face face, FXFT_Matrix* matrix)
    : m_Face(face) {
  FXFT_Set_Transform(m_Face, matrix, 0);
}

ScopedFontTransform::~ScopedFontTransform() {
  ResetTransform(m_Face);
}
