// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2009 ZXing authors
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

#include <algorithm>
#include <cwctype>

#include "core/fxge/cfx_fxgedevice.h"
#include "core/fxge/cfx_gemodule.h"
#include "fxbarcode/BC_Writer.h"
#include "fxbarcode/oned/BC_OneDimWriter.h"
#include "fxbarcode/oned/BC_OnedEAN13Writer.h"

namespace {

const int32_t FIRST_DIGIT_ENCODINGS[10] = {0x00, 0x0B, 0x0D, 0xE,  0x13,
                                           0x19, 0x1C, 0x15, 0x16, 0x1A};
const int32_t START_END_PATTERN[3] = {1, 1, 1};
const int32_t MIDDLE_PATTERN[5] = {1, 1, 1, 1, 1};
const int32_t L_PATTERNS[10][4] = {
    {3, 2, 1, 1}, {2, 2, 2, 1}, {2, 1, 2, 2}, {1, 4, 1, 1}, {1, 1, 3, 2},
    {1, 2, 3, 1}, {1, 1, 1, 4}, {1, 3, 1, 2}, {1, 2, 1, 3}, {3, 1, 1, 2}};
const int32_t L_AND_G_PATTERNS[20][4] = {
    {3, 2, 1, 1}, {2, 2, 2, 1}, {2, 1, 2, 2}, {1, 4, 1, 1}, {1, 1, 3, 2},
    {1, 2, 3, 1}, {1, 1, 1, 4}, {1, 3, 1, 2}, {1, 2, 1, 3}, {3, 1, 1, 2},
    {1, 1, 2, 3}, {1, 2, 2, 2}, {2, 2, 1, 2}, {1, 1, 4, 1}, {2, 3, 1, 1},
    {1, 3, 2, 1}, {4, 1, 1, 1}, {2, 1, 3, 1}, {3, 1, 2, 1}, {2, 1, 1, 3}};

}  // namespace

CBC_OnedEAN13Writer::CBC_OnedEAN13Writer() {
  m_bLeftPadding = true;
  m_codeWidth = 3 + (7 * 6) + 5 + (7 * 6) + 3;
}
CBC_OnedEAN13Writer::~CBC_OnedEAN13Writer() {}

bool CBC_OnedEAN13Writer::CheckContentValidity(
    const CFX_WideStringC& contents) {
  return std::all_of(contents.begin(), contents.end(), std::iswdigit);
}

CFX_WideString CBC_OnedEAN13Writer::FilterContents(
    const CFX_WideStringC& contents) {
  CFX_WideString filtercontents;
  wchar_t ch;
  for (int32_t i = 0; i < contents.GetLength(); i++) {
    ch = contents.GetAt(i);
    if (ch > 175) {
      i++;
      continue;
    }
    if (ch >= '0' && ch <= '9') {
      filtercontents += ch;
    }
  }
  return filtercontents;
}
int32_t CBC_OnedEAN13Writer::CalcChecksum(const CFX_ByteString& contents) {
  int32_t odd = 0;
  int32_t even = 0;
  int32_t j = 1;
  for (int32_t i = contents.GetLength() - 1; i >= 0; i--) {
    if (j % 2) {
      odd += FXSYS_atoi(contents.Mid(i, 1).c_str());
    } else {
      even += FXSYS_atoi(contents.Mid(i, 1).c_str());
    }
    j++;
  }
  int32_t checksum = (odd * 3 + even) % 10;
  checksum = (10 - checksum) % 10;
  return (checksum);
}
uint8_t* CBC_OnedEAN13Writer::Encode(const CFX_ByteString& contents,
                                     BCFORMAT format,
                                     int32_t& outWidth,
                                     int32_t& outHeight,
                                     int32_t& e) {
  uint8_t* ret = Encode(contents, format, outWidth, outHeight, 0, e);
  if (e != BCExceptionNO)
    return nullptr;
  return ret;
}
uint8_t* CBC_OnedEAN13Writer::Encode(const CFX_ByteString& contents,
                                     BCFORMAT format,
                                     int32_t& outWidth,
                                     int32_t& outHeight,
                                     int32_t hints,
                                     int32_t& e) {
  if (format != BCFORMAT_EAN_13) {
    e = BCExceptionOnlyEncodeEAN_13;
  }
  uint8_t* ret =
      CBC_OneDimWriter::Encode(contents, format, outWidth, outHeight, hints, e);
  if (e != BCExceptionNO)
    return nullptr;
  return ret;
}
uint8_t* CBC_OnedEAN13Writer::Encode(const CFX_ByteString& contents,
                                     int32_t& outLength,
                                     int32_t& e) {
  if (contents.GetLength() != 13) {
    e = BCExceptionDigitLengthShould13;
    return nullptr;
  }
  m_iDataLenth = 13;
  int32_t firstDigit = FXSYS_atoi(contents.Mid(0, 1).c_str());
  int32_t parities = FIRST_DIGIT_ENCODINGS[firstDigit];
  outLength = m_codeWidth;
  uint8_t* result = FX_Alloc(uint8_t, m_codeWidth);
  int32_t pos = 0;
  pos += AppendPattern(result, pos, START_END_PATTERN, 3, 1, e);
  if (e != BCExceptionNO) {
    FX_Free(result);
    return nullptr;
  }
  int32_t i = 0;
  for (i = 1; i <= 6; i++) {
    int32_t digit = FXSYS_atoi(contents.Mid(i, 1).c_str());
    if ((parities >> (6 - i) & 1) == 1) {
      digit += 10;
    }
    pos += AppendPattern(result, pos, L_AND_G_PATTERNS[digit], 4, 0, e);
    if (e != BCExceptionNO) {
      FX_Free(result);
      return nullptr;
    }
  }
  pos += AppendPattern(result, pos, MIDDLE_PATTERN, 5, 0, e);
  if (e != BCExceptionNO) {
    FX_Free(result);
    return nullptr;
  }
  for (i = 7; i <= 12; i++) {
    int32_t digit = FXSYS_atoi(contents.Mid(i, 1).c_str());
    pos += AppendPattern(result, pos, L_PATTERNS[digit], 4, 1, e);
    if (e != BCExceptionNO) {
      FX_Free(result);
      return nullptr;
    }
  }
  pos += AppendPattern(result, pos, START_END_PATTERN, 3, 1, e);
  if (e != BCExceptionNO) {
    FX_Free(result);
    return nullptr;
  }
  return result;
}

void CBC_OnedEAN13Writer::ShowChars(
    const CFX_WideStringC& contents,
    CFX_RenderDevice* device,
    const CFX_Matrix* matrix,
    int32_t barWidth,
    int32_t multiple,
    int32_t& e) {
  if (!device) {
    e = BCExceptionIllegalArgument;
    return;
  }
  int32_t leftPadding = 7 * multiple;
  int32_t leftPosition = 3 * multiple + leftPadding;
  CFX_ByteString str = FX_UTF8Encode(contents);
  int32_t iLen = str.GetLength();
  FXTEXT_CHARPOS* pCharPos = FX_Alloc(FXTEXT_CHARPOS, iLen);
  memset(pCharPos, 0, sizeof(FXTEXT_CHARPOS) * iLen);

  int32_t iFontSize = (int32_t)fabs(m_fFontSize);
  int32_t iTextHeight = iFontSize + 1;
  CFX_ByteString tempStr = str.Mid(1, 6);
  int32_t strWidth = multiple * 42;

  CFX_Matrix matr(m_outputHScale, 0.0, 0.0, 1.0, 0.0, 0.0);
  CFX_FloatRect rect((float)leftPosition, (float)(m_Height - iTextHeight),
                     (float)(leftPosition + strWidth - 0.5), (float)m_Height);
  matr.Concat(*matrix);
  matr.TransformRect(rect);
  FX_RECT re = rect.GetOuterRect();
  device->FillRect(&re, m_backgroundColor);
  CFX_FloatRect rect1(
      (float)(leftPosition + 47 * multiple), (float)(m_Height - iTextHeight),
      (float)(leftPosition + 47 * multiple + strWidth - 0.5), (float)m_Height);
  CFX_Matrix matr1(m_outputHScale, 0.0, 0.0, 1.0, 0.0, 0.0);
  matr1.Concat(*matrix);
  matr1.TransformRect(rect1);
  re = rect1.GetOuterRect();
  device->FillRect(&re, m_backgroundColor);
  int32_t strWidth1 = multiple * 7;
  CFX_Matrix matr2(m_outputHScale, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
  CFX_FloatRect rect2(0.0f, (float)(m_Height - iTextHeight),
                      (float)strWidth1 - 0.5f, (float)m_Height);
  matr2.Concat(*matrix);
  matr2.TransformRect(rect2);
  re = rect2.GetOuterRect();
  device->FillRect(&re, m_backgroundColor);

  float blank = 0.0;
  iLen = tempStr.GetLength();
  strWidth = (int32_t)(strWidth * m_outputHScale);

  CalcTextInfo(tempStr, pCharPos + 1, m_pFont, (float)strWidth, iFontSize,
               blank);
  {
    CFX_Matrix affine_matrix1(1.0, 0.0, 0.0, -1.0,
                              (float)leftPosition * m_outputHScale,
                              (float)(m_Height - iTextHeight) + iFontSize);
    if (matrix)
      affine_matrix1.Concat(*matrix);
    device->DrawNormalText(iLen, pCharPos + 1, m_pFont,
                           static_cast<float>(iFontSize), &affine_matrix1,
                           m_fontColor, FXTEXT_CLEARTYPE);
  }
  tempStr = str.Mid(7, 6);
  iLen = tempStr.GetLength();
  CalcTextInfo(tempStr, pCharPos + 7, m_pFont, (float)strWidth, iFontSize,
               blank);
  {
    CFX_Matrix affine_matrix1(
        1.0, 0.0, 0.0, -1.0,
        (float)(leftPosition + 47 * multiple) * m_outputHScale,
        (float)(m_Height - iTextHeight + iFontSize));
    if (matrix)
      affine_matrix1.Concat(*matrix);
    device->DrawNormalText(iLen, pCharPos + 7, m_pFont,
                           static_cast<float>(iFontSize), &affine_matrix1,
                           m_fontColor, FXTEXT_CLEARTYPE);
  }
  tempStr = str.Mid(0, 1);
  iLen = tempStr.GetLength();
  strWidth = multiple * 7;
  strWidth = (int32_t)(strWidth * m_outputHScale);

  CalcTextInfo(tempStr, pCharPos, m_pFont, (float)strWidth, iFontSize, blank);
  {
    CFX_Matrix affine_matrix1(1.0, 0.0, 0.0, -1.0, 0.0,
                              (float)(m_Height - iTextHeight + iFontSize));
    if (matrix)
      affine_matrix1.Concat(*matrix);
    device->DrawNormalText(iLen, pCharPos, m_pFont,
                           static_cast<float>(iFontSize), &affine_matrix1,
                           m_fontColor, FXTEXT_CLEARTYPE);
  }
  FX_Free(pCharPos);
}

void CBC_OnedEAN13Writer::RenderResult(const CFX_WideStringC& contents,
                                       uint8_t* code,
                                       int32_t codeLength,
                                       bool isDevice,
                                       int32_t& e) {
  CBC_OneDimWriter::RenderResult(contents, code, codeLength, isDevice, e);
}
