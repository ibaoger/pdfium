// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_devicecs.h"

#include <limits.h>

#include <algorithm>

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxcodec/fx_codec.h"
#include "third_party/base/logging.h"
#include "third_party/base/stl_util.h"

namespace {

float NormalizeChannel(float fVal) {
  return pdfium::clamp(fVal, 0.0f, 1.0f);
}

}  // namespace

uint32_t ComponentsForFamily(int family) {
  if (family == PDFCS_DEVICERGB)
    return 3;
  if (family == PDFCS_DEVICEGRAY)
    return 1;
  ASSERT(family == PDFCS_DEVICECMYK);
  return 4;
}

void sRGB_to_AdobeCMYK(float R,
                       float G,
                       float B,
                       float& c,
                       float& m,
                       float& y,
                       float& k) {
  c = 1.0f - R;
  m = 1.0f - G;
  y = 1.0f - B;
  k = std::min(c, std::min(m, y));
}

void ReverseRGB(uint8_t* pDestBuf, const uint8_t* pSrcBuf, int pixels) {
  if (pDestBuf == pSrcBuf) {
    for (int i = 0; i < pixels; i++) {
      uint8_t temp = pDestBuf[2];
      pDestBuf[2] = pDestBuf[0];
      pDestBuf[0] = temp;
      pDestBuf += 3;
    }
  } else {
    for (int i = 0; i < pixels; i++) {
      *pDestBuf++ = pSrcBuf[2];
      *pDestBuf++ = pSrcBuf[1];
      *pDestBuf++ = pSrcBuf[0];
      pSrcBuf += 3;
    }
  }
}

CPDF_DeviceCS::CPDF_DeviceCS(int family)
    : CPDF_ColorSpace(nullptr, family, ComponentsForFamily(family)) {
  ASSERT(family == PDFCS_DEVICEGRAY || family == PDFCS_DEVICERGB ||
         family == PDFCS_DEVICECMYK);
}

CPDF_DeviceCS::~CPDF_DeviceCS() {}

pdfium::Optional<std::tuple<float, float, float>> CPDF_DeviceCS::GetRGB(
    float* pBuf) const {
  switch (m_Family) {
    case PDFCS_DEVICEGRAY:
      return std::make_tuple(NormalizeChannel(*pBuf), NormalizeChannel(*pBuf),
                             NormalizeChannel(*pBuf));
    case PDFCS_DEVICERGB:
      return std::make_tuple(NormalizeChannel(pBuf[0]),
                             NormalizeChannel(pBuf[1]),
                             NormalizeChannel(pBuf[2]));
    case PDFCS_DEVICECMYK:
      if (m_dwStdConversion) {
        float k = pBuf[3];
        return std::make_tuple(1.0f - std::min(1.0f, pBuf[0] + k),
                               1.0f - std::min(1.0f, pBuf[1] + k),
                               1.0f - std::min(1.0f, pBuf[2] + k));
      } else {
        return AdobeCMYK_to_sRGB(
            NormalizeChannel(pBuf[0]), NormalizeChannel(pBuf[1]),
            NormalizeChannel(pBuf[2]), NormalizeChannel(pBuf[3]));
      }
    default:
      NOTREACHED();
      return pdfium::Optional<std::tuple<float, float, float>>();
  }
}

void CPDF_DeviceCS::TranslateImageLine(uint8_t* pDestBuf,
                                       const uint8_t* pSrcBuf,
                                       int pixels,
                                       int image_width,
                                       int image_height,
                                       bool bTransMask) const {
  switch (m_Family) {
    case PDFCS_DEVICEGRAY:
      for (int i = 0; i < pixels; i++) {
        *pDestBuf++ = pSrcBuf[i];
        *pDestBuf++ = pSrcBuf[i];
        *pDestBuf++ = pSrcBuf[i];
      }
      break;
    case PDFCS_DEVICERGB:
      ReverseRGB(pDestBuf, pSrcBuf, pixels);
      break;
    case PDFCS_DEVICECMYK:
      if (bTransMask) {
        for (int i = 0; i < pixels; i++) {
          int k = 255 - pSrcBuf[3];
          pDestBuf[0] = ((255 - pSrcBuf[0]) * k) / 255;
          pDestBuf[1] = ((255 - pSrcBuf[1]) * k) / 255;
          pDestBuf[2] = ((255 - pSrcBuf[2]) * k) / 255;
          pDestBuf += 3;
          pSrcBuf += 4;
        }
      } else {
        for (int i = 0; i < pixels; i++) {
          if (m_dwStdConversion) {
            uint8_t k = pSrcBuf[3];
            pDestBuf[2] = 255 - std::min(255, pSrcBuf[0] + k);
            pDestBuf[1] = 255 - std::min(255, pSrcBuf[1] + k);
            pDestBuf[0] = 255 - std::min(255, pSrcBuf[2] + k);
          } else {
            std::tie(pDestBuf[2], pDestBuf[1], pDestBuf[0]) =
                AdobeCMYK_to_sRGB1(pSrcBuf[0], pSrcBuf[1], pSrcBuf[2],
                                   pSrcBuf[3]);
          }
          pSrcBuf += 4;
          pDestBuf += 3;
        }
      }
      break;
    default:
      NOTREACHED();
      break;
  }
}
