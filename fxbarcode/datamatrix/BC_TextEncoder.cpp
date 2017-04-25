// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2006-2007 Jeremias Maerki.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fxbarcode/datamatrix/BC_TextEncoder.h"

#include "core/fxcrt/fx_extension.h"
#include "fxbarcode/BC_Dimension.h"
#include "fxbarcode/common/BC_CommonBitMatrix.h"
#include "fxbarcode/datamatrix/BC_Encoder.h"
#include "fxbarcode/datamatrix/BC_EncoderContext.h"
#include "fxbarcode/datamatrix/BC_HighLevelEncoder.h"
#include "fxbarcode/datamatrix/BC_SymbolInfo.h"
#include "fxbarcode/datamatrix/BC_SymbolShapeHint.h"

CBC_TextEncoder::CBC_TextEncoder() {}
CBC_TextEncoder::~CBC_TextEncoder() {}

int32_t CBC_TextEncoder::getEncodingMode() {
  return TEXT_ENCODATION;
}

int32_t CBC_TextEncoder::encodeChar(wchar_t c, CFX_WideString& sb, int32_t& e) {
  if (c == ' ') {
    sb += L'\3';
    return 1;
  }
  if (std::iswdigit(c)) {
    sb += (c - '0' + 4);
    return 1;
  }
  if (FXSYS_islower(c)) {
    sb += c - 'a' + 14;
    return 1;
  }
  if (c <= 0x1f) {
    sb += L'\0';
    sb += c;
    return 2;
  }
  if (c >= '!' && c <= '/') {
    sb += L'\1';
    sb += c - '!';
    return 2;
  }
  if (c >= ':' && c <= '@') {
    sb += L'\1';
    sb += c - ':' + 15;
    return 2;
  }
  if (c >= '[' && c <= '_') {
    sb += L'\1';
    sb += c - '[' + 22;
    return 2;
  }
  if (c == 0x0060) {
    sb += L'\2';
    sb += c - 96;
    return 2;
  }
  if (FXSYS_isupper(c)) {
    sb += L'\2';
    sb += c - 'A' + 1;
    return 2;
  }
  if (c >= '{' && c <= 0x007f) {
    sb += L'\2';
    sb += (c - '{' + 27);
    return 2;
  }
  if (c >= 0x0080) {
    sb += L'\1';
    sb += 0x001e;
    int32_t len = 2;
    len += encodeChar(c - 128, sb, e);
    if (e != BCExceptionNO)
      return -1;
    return len;
  }
  CBC_HighLevelEncoder::illegalCharacter(c, e);
  return -1;
}
