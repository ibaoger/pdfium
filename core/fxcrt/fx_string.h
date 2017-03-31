// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_STRING_H_
#define CORE_FXCRT_FX_STRING_H_

#include <stdint.h>  // For intptr_t.

#include <algorithm>
#include <functional>

#include "core/fxcrt/cfx_bytestring.h"
#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/cfx_string_c_template.h"
#include "core/fxcrt/cfx_string_data_template.h"
#include "core/fxcrt/cfx_widestring.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"

#define FXBSTR_ID(c1, c2, c3, c4)                                      \
  (((uint32_t)c1 << 24) | ((uint32_t)c2 << 16) | ((uint32_t)c3 << 8) | \
   ((uint32_t)c4))

CFX_ByteString FX_UTF8Encode(const CFX_WideStringC& wsStr);
float FX_atof(const CFX_ByteStringC& str);
inline float FX_atof(const CFX_WideStringC& wsStr) {
  return FX_atof(FX_UTF8Encode(wsStr).c_str());
}
bool FX_atonum(const CFX_ByteStringC& str, void* pData);
FX_STRSIZE FX_ftoa(float f, char* buf);

#endif  // CORE_FXCRT_FX_STRING_H_
