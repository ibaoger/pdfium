// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_QRCODE_BC_QRCODERECBLOCKSDATA_H_
#define FXBARCODE_QRCODE_BC_QRCODERECBLOCKSDATA_H_

#include "core/fxcrt/fx_basic.h"

struct CBC_QRCoderECBlockData {
  int ecCodeWordsPerBlock;
  int count1;
  int dataCodeWords1;
  int count2;
  int dataCodeWords2;
};

extern const CBC_QRCoderECBlockData g_ECBData[40][4];

#endif  // FXBARCODE_QRCODE_BC_QRCODERECBLOCKSDATA_H_
