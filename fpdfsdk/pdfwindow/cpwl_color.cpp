// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pdfwindow/cpwl_color.h"

#include <algorithm>

namespace {

CPWL_Color ConvertCMYK2GRAY(FX_FLOAT dC,
                            FX_FLOAT dM,
                            FX_FLOAT dY,
                            FX_FLOAT dK) {
  if (dC < 0 || dC > 1 || dM < 0 || dM > 1 || dY < 0 || dY > 1 || dK < 0 ||
      dK > 1) {
    return CPWL_Color(COLORTYPE_GRAY);
  }
  return CPWL_Color(
      COLORTYPE_GRAY,
      1.0f - std::min(1.0f, 0.3f * dC + 0.59f * dM + 0.11f * dY + dK));
}

CPWL_Color ConvertGRAY2CMYK(FX_FLOAT dGray) {
  if (dGray < 0 || dGray > 1)
    return CPWL_Color(COLORTYPE_CMYK);
  return CPWL_Color(COLORTYPE_CMYK, 0.0f, 0.0f, 0.0f, 1.0f - dGray);
}

CPWL_Color ConvertGRAY2RGB(FX_FLOAT dGray) {
  if (dGray < 0 || dGray > 1)
    return CPWL_Color(COLORTYPE_RGB);
  return CPWL_Color(COLORTYPE_RGB, dGray, dGray, dGray);
}

CPWL_Color ConvertRGB2GRAY(FX_FLOAT dR, FX_FLOAT dG, FX_FLOAT dB) {
  if (dR < 0 || dR > 1 || dG < 0 || dG > 0 || dB < 0 || dB > 1)
    return CPWL_Color(COLORTYPE_GRAY);
  return CPWL_Color(COLORTYPE_GRAY, 0.3f * dR + 0.59f * dG + 0.11f * dB);
}

CPWL_Color ConvertCMYK2RGB(FX_FLOAT dC, FX_FLOAT dM, FX_FLOAT dY, FX_FLOAT dK) {
  if (dC < 0 || dC > 1 || dM < 0 || dM > 1 || dY < 0 || dY > 1 || dK < 0 ||
      dK > 1) {
    return CPWL_Color(COLORTYPE_RGB);
  }
  return CPWL_Color(COLORTYPE_RGB, 1.0f - std::min(1.0f, dC + dK),
                    1.0f - std::min(1.0f, dM + dK),
                    1.0f - std::min(1.0f, dY + dK));
}

CPWL_Color ConvertRGB2CMYK(FX_FLOAT dR, FX_FLOAT dG, FX_FLOAT dB) {
  if (dR < 0 || dR > 1 || dG < 0 || dG > 1 || dB < 0 || dB > 1)
    return CPWL_Color(COLORTYPE_CMYK);

  FX_FLOAT c = 1.0f - dR;
  FX_FLOAT m = 1.0f - dG;
  FX_FLOAT y = 1.0f - dB;
  return CPWL_Color(COLORTYPE_CMYK, c, m, y, std::min(c, std::min(m, y)));
}

}  // namespace

CPWL_Color CPWL_Color::ConvertColorType(int32_t nConvertColorType) const {
  if (nColorType == nConvertColorType)
    return *this;

  CPWL_Color ret;
  switch (nColorType) {
    case COLORTYPE_TRANSPARENT:
      ret = *this;
      ret.nColorType = COLORTYPE_TRANSPARENT;
      break;
    case COLORTYPE_GRAY:
      switch (nConvertColorType) {
        case COLORTYPE_RGB:
          ret = ConvertGRAY2RGB(fColor1);
          break;
        case COLORTYPE_CMYK:
          ret = ConvertGRAY2CMYK(fColor1);
          break;
      }
      break;
    case COLORTYPE_RGB:
      switch (nConvertColorType) {
        case COLORTYPE_GRAY:
          ret = ConvertRGB2GRAY(fColor1, fColor2, fColor3);
          break;
        case COLORTYPE_CMYK:
          ret = ConvertRGB2CMYK(fColor1, fColor2, fColor3);
          break;
      }
      break;
    case COLORTYPE_CMYK:
      switch (nConvertColorType) {
        case COLORTYPE_GRAY:
          ret = ConvertCMYK2GRAY(fColor1, fColor2, fColor3, fColor4);
          break;
        case COLORTYPE_RGB:
          ret = ConvertCMYK2RGB(fColor1, fColor2, fColor3, fColor4);
          break;
      }
      break;
  }
  return ret;
}

FX_COLORREF CPWL_Color::ToFXColor(int32_t nTransparency) const {
  CPWL_Color ret;
  switch (nColorType) {
    case COLORTYPE_TRANSPARENT: {
      ret = CPWL_Color(COLORTYPE_TRANSPARENT, 0, 0, 0, nTransparency);
      break;
    }
    case COLORTYPE_GRAY: {
      ret = ConvertGRAY2RGB(fColor1);
      break;
    }
    case COLORTYPE_RGB: {
      ret = CPWL_Color(COLORTYPE_RGB, fColor1, fColor2, fColor3);
      break;
    }
    case COLORTYPE_CMYK: {
      ret = ConvertCMYK2RGB(fColor1, fColor2, fColor3, fColor4);
      break;
    }
  }
  return ArgbEncode(ret.fColor4, static_cast<int32_t>(ret.fColor1 * 255),
                    static_cast<int32_t>(ret.fColor2 * 255),
                    static_cast<int32_t>(ret.fColor3 * 255));
}
