// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_FGAS_UNICODE_H_
#define XFA_FGAS_LAYOUT_FGAS_UNICODE_H_

#include "xfa/fgas/crt/fgas_utils.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"

struct FX_TPO {
  int32_t index;
  int32_t pos;

  bool operator<(const FX_TPO& that) const { return pos < that.pos; }
};

#endif  // XFA_FGAS_LAYOUT_FGAS_UNICODE_H_
