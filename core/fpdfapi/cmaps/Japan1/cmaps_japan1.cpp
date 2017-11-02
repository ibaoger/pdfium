// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/cmaps/Japan1/cmaps_japan1.h"

#include "core/fpdfapi/cmaps/cmap_int.h"
#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/font/cpdf_fontglobals.h"
#include "core/fpdfapi/page/cpdf_pagemodule.h"

static const FXCMAP_CMap g_FXCMAP_Japan1_cmaps[] = {
    {"83pv-RKSJ-H", FXCMAP_CMap::Range, g_FXCMAP_83pv_RKSJ_H_1, 222, nullptr, 0,
     0},
    {"90ms-RKSJ-H", FXCMAP_CMap::Range, g_FXCMAP_90ms_RKSJ_H_2, 171, nullptr, 0,
     0},
    {"90ms-RKSJ-V", FXCMAP_CMap::Range, g_FXCMAP_90ms_RKSJ_V_2, 78, nullptr, 0,
     -1},
    {"90msp-RKSJ-H", FXCMAP_CMap::Range, g_FXCMAP_90msp_RKSJ_H_2, 170, nullptr,
     0, -2},
    {"90msp-RKSJ-V", FXCMAP_CMap::Range, g_FXCMAP_90msp_RKSJ_V_2, 78, nullptr,
     0, -1},
    {"90pv-RKSJ-H", FXCMAP_CMap::Range, g_FXCMAP_90pv_RKSJ_H_1, 263, nullptr, 0,
     0},
    {"Add-RKSJ-H", FXCMAP_CMap::Range, g_FXCMAP_Add_RKSJ_H_1, 635, nullptr, 0,
     0},
    {"Add-RKSJ-V", FXCMAP_CMap::Range, g_FXCMAP_Add_RKSJ_V_1, 57, nullptr, 0,
     -1},
    {"EUC-H", FXCMAP_CMap::Range, g_FXCMAP_EUC_H_1, 120, nullptr, 0, 0},
    {"EUC-V", FXCMAP_CMap::Range, g_FXCMAP_EUC_V_1, 27, nullptr, 0, -1},
    {"Ext-RKSJ-H", FXCMAP_CMap::Range, g_FXCMAP_Ext_RKSJ_H_2, 665, nullptr, 0,
     -4},
    {"Ext-RKSJ-V", FXCMAP_CMap::Range, g_FXCMAP_Ext_RKSJ_V_2, 39, nullptr, 0,
     -1},
    {"H", FXCMAP_CMap::Range, g_FXCMAP_H_1, 118, nullptr, 0, 0},
    {"V", FXCMAP_CMap::Range, g_FXCMAP_V_1, 27, nullptr, 0, -1},
    {"UniJIS-UCS2-H", FXCMAP_CMap::Single, g_FXCMAP_UniJIS_UCS2_H_4, 9772,
     nullptr, 0, 0},
    {"UniJIS-UCS2-V", FXCMAP_CMap::Single, g_FXCMAP_UniJIS_UCS2_V_4, 251,
     nullptr, 0, -1},
    {"UniJIS-UCS2-HW-H", FXCMAP_CMap::Range, g_FXCMAP_UniJIS_UCS2_HW_H_4, 4,
     nullptr, 0, -2},
    {"UniJIS-UCS2-HW-V", FXCMAP_CMap::Range, g_FXCMAP_UniJIS_UCS2_HW_V_4, 199,
     nullptr, 0, -1},
    {"UniJIS-UTF16-H", FXCMAP_CMap::Single, g_FXCMAP_UniJIS_UCS2_H_4, 9772,
     nullptr, 0, 0},
    {"UniJIS-UTF16-V", FXCMAP_CMap::Single, g_FXCMAP_UniJIS_UCS2_V_4, 251,
     nullptr, 0, -1},
};

void CPDF_ModuleMgr::LoadEmbeddedJapan1CMaps() {
  CPDF_FontGlobals* pFontGlobals =
      CPDF_ModuleMgr::Get()->GetPageModule()->GetFontGlobals();
  pFontGlobals->SetEmbeddedCharset(CIDSET_JAPAN1, g_FXCMAP_Japan1_cmaps,
                                   FX_ArraySize(g_FXCMAP_Japan1_cmaps));
  pFontGlobals->SetEmbeddedToUnicode(CIDSET_JAPAN1,
                                     g_FXCMAP_Japan1CID2Unicode_4, 15444);
}
