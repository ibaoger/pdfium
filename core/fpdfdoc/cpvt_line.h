// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPVT_LINE_H_
#define CORE_FPDFDOC_CPVT_LINE_H_

#include "core/fpdfdoc/cpvt_wordplace.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"

struct CPVT_Line {
  CPVT_Line();

  CPVT_WordPlace lineplace;
  CPVT_WordPlace lineEnd;
  CFX_PointF ptLine;
  FX_FLOAT fLineWidth;
  FX_FLOAT fLineAscent;
  FX_FLOAT fLineDescent;
};

#endif  // CORE_FPDFDOC_CPVT_LINE_H_
