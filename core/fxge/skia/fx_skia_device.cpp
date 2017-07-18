// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <utility>
#include <vector>

#include "core/fpdfapi/page/cpdf_expintfunc.h"
#include "core/fpdfapi/page/cpdf_function.h"
#include "core/fpdfapi/page/cpdf_meshstream.h"
#include "core/fpdfapi/page/cpdf_sampledfunc.h"
#include "core/fpdfapi/page/cpdf_shadingpattern.h"
#include "core/fpdfapi/page/cpdf_stitchfunc.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/dib/cfx_bitmapcomposer.h"
#include "core/fxge/dib/cfx_imagerenderer.h"
#include "core/fxge/dib/cfx_imagestretcher.h"
#include "core/fxge/skia/fx_skia_device.h"
#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkClipOp.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkRSXform.h"
#include "third_party/skia/include/core/SkShader.h"
#include "third_party/skia/include/core/SkStream.h"
#include "third_party/skia/include/core/SkTypeface.h"
#include "third_party/skia/include/effects/SkDashPathEffect.h"
#include "third_party/skia/include/effects/SkGradientShader.h"
#include "third_party/skia/include/pathops/SkPathOps.h"

#ifdef _SKIA_SUPPORT_PATHS_
#include "core/fxge/cfx_cliprgn.h"
#endif  // _SKIA_SUPPORT_PATHS_

#ifdef _SKIA_SUPPORT_
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkColorPriv.h"
#include "third_party/skia/include/core/SkMaskFilter.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"
#endif  // _SKIA_SUPPORT_

namespace {

#ifdef _SKIA_SUPPORT_PATHS_
void RgbByteOrderTransferBitmap(const CFX_RetainPtr<CFX_DIBitmap>& pBitmap,
                                int dest_left,
                                int dest_top,
                                int width,
                                int height,
                                const CFX_RetainPtr<CFX_DIBSource>& pSrcBitmap,
                                int src_left,
                                int src_top) {
  if (!pBitmap)
    return;

  pBitmap->GetOverlapRect(dest_left, dest_top, width, height,
                          pSrcBitmap->GetWidth(), pSrcBitmap->GetHeight(),
                          src_left, src_top, nullptr);
  if (width == 0 || height == 0)
    return;

  int Bpp = pBitmap->GetBPP() / 8;
  FXDIB_Format dest_format = pBitmap->GetFormat();
  FXDIB_Format src_format = pSrcBitmap->GetFormat();
  int pitch = pBitmap->GetPitch();
  uint8_t* buffer = pBitmap->GetBuffer();
  if (dest_format == src_format) {
    for (int row = 0; row < height; row++) {
      uint8_t* dest_scan = buffer + (dest_top + row) * pitch + dest_left * Bpp;
      uint8_t* src_scan =
          (uint8_t*)pSrcBitmap->GetScanline(src_top + row) + src_left * Bpp;
      if (Bpp == 4) {
        for (int col = 0; col < width; col++) {
          FXARGB_SETDIB(dest_scan, FXARGB_MAKE(src_scan[3], src_scan[0],
                                               src_scan[1], src_scan[2]));
          dest_scan += 4;
          src_scan += 4;
        }
      } else {
        for (int col = 0; col < width; col++) {
          *dest_scan++ = src_scan[2];
          *dest_scan++ = src_scan[1];
          *dest_scan++ = src_scan[0];
          src_scan += 3;
        }
      }
    }
    return;
  }

  uint8_t* dest_buf = buffer + dest_top * pitch + dest_left * Bpp;
  if (dest_format == FXDIB_Rgb) {
    if (src_format == FXDIB_Rgb32) {
      for (int row = 0; row < height; row++) {
        uint8_t* dest_scan = dest_buf + row * pitch;
        uint8_t* src_scan =
            (uint8_t*)pSrcBitmap->GetScanline(src_top + row) + src_left * 4;
        for (int col = 0; col < width; col++) {
          *dest_scan++ = src_scan[2];
          *dest_scan++ = src_scan[1];
          *dest_scan++ = src_scan[0];
          src_scan += 4;
        }
      }
    } else {
      NOTREACHED();
    }
    return;
  }

  if (dest_format == FXDIB_Argb || dest_format == FXDIB_Rgb32) {
    if (src_format == FXDIB_Rgb) {
      for (int row = 0; row < height; row++) {
        uint8_t* dest_scan = (uint8_t*)(dest_buf + row * pitch);
        uint8_t* src_scan =
            (uint8_t*)pSrcBitmap->GetScanline(src_top + row) + src_left * 3;
        for (int col = 0; col < width; col++) {
          FXARGB_SETDIB(dest_scan, FXARGB_MAKE(0xff, src_scan[0], src_scan[1],
                                               src_scan[2]));
          dest_scan += 4;
          src_scan += 3;
        }
      }
    } else if (src_format == FXDIB_Rgb32) {
      ASSERT(dest_format == FXDIB_Argb);
      for (int row = 0; row < height; row++) {
        uint8_t* dest_scan = dest_buf + row * pitch;
        uint8_t* src_scan =
            (uint8_t*)(pSrcBitmap->GetScanline(src_top + row) + src_left * 4);
        for (int col = 0; col < width; col++) {
          FXARGB_SETDIB(dest_scan, FXARGB_MAKE(0xff, src_scan[0], src_scan[1],
                                               src_scan[2]));
          src_scan += 4;
          dest_scan += 4;
        }
      }
    }
    return;
  }

  NOTREACHED();
}

#endif  // _SKIA_SUPPORT_PATHS_

#define SHOW_SKIA_PATH 0  // set to 1 to print the path contents
#if SHOW_SKIA_PATH
#define SHOW_SKIA_PATH_SHORTHAND 0  // set to 1 for abbreviated path contents
#endif
#define DRAW_SKIA_CLIP 0  // set to 1 to draw a green rectangle around the clip
#define SHOW_TEXT_GLYPHS 0  // set to 1 to print unichar equivalent of glyph

#if SHOW_SKIA_PATH
void DebugShowSkiaPaint(const SkPaint& paint) {
  if (SkPaint::kFill_Style == paint.getStyle()) {
    printf("fill 0x%08x\n", paint.getColor());
  } else {
    printf("stroke 0x%08x width %g\n", paint.getColor(),
           paint.getStrokeWidth());
  }
}
#endif  // SHOW_SKIA_PATH

void DebugShowSkiaPath(const SkPath& path) {
#if SHOW_SKIA_PATH
#if SHOW_SKIA_PATH_SHORTHAND
  printf(" **\n");
#else
  SkDynamicMemoryWStream stream;
  path.dump(&stream, false, false);
  std::unique_ptr<char, FxFreeDeleter> storage;
  storage.reset(FX_Alloc(char, stream.bytesWritten()));
  stream.copyTo(storage.get());
  printf("%.*s", (int)stream.bytesWritten(), storage.get());
#endif  // SHOW_SKIA_PATH_SHORTHAND
#endif  // SHOW_SKIA_PATH
}

void DebugShowCanvasClip(CFX_SkiaDeviceDriver* driver, const SkCanvas* canvas) {
#if SHOW_SKIA_PATH
  SkMatrix matrix = canvas->getTotalMatrix();
  SkScalar m[9];
  matrix.get9(m);
  printf("matrix (%g,%g,%g) (%g,%g,%g) (%g,%g,%g)\n", m[0], m[1], m[2], m[3],
         m[4], m[5], m[6], m[7], m[8]);
  SkRect local = canvas->getLocalClipBounds();
  SkIRect device = canvas->getDeviceClipBounds();

  printf("local bounds %g %g %g %g\n", local.fLeft, local.fTop, local.fRight,
         local.fBottom);
  printf("device bounds %d %d %d %d\n", device.fLeft, device.fTop,
         device.fRight, device.fBottom);
  FX_RECT clipBox;
  driver->GetClipBox(&clipBox);
  printf("reported bounds %d %d %d %d\n", clipBox.left, clipBox.top,
         clipBox.right, clipBox.bottom);
#endif  // SHOW_SKIA_PATH
}

void DebugShowSkiaDrawPath(CFX_SkiaDeviceDriver* driver,
                           const SkCanvas* canvas,
                           const SkPaint& paint,
                           const SkPath& path) {
#if SHOW_SKIA_PATH
  DebugShowSkiaPaint(paint);
  DebugShowCanvasClip(driver, canvas);
  DebugShowSkiaPath(path);
  printf("\n");
#endif  // SHOW_SKIA_PATH
}

void DebugShowSkiaDrawRect(CFX_SkiaDeviceDriver* driver,
                           const SkCanvas* canvas,
                           const SkPaint& paint,
                           const SkRect& rect) {
#if SHOW_SKIA_PATH
  DebugShowSkiaPaint(paint);
  DebugShowCanvasClip(driver, canvas);
  printf("rect %g %g %g %g\n", rect.fLeft, rect.fTop, rect.fRight,
         rect.fBottom);
#endif  // SHOW_SKIA_PATH
}

#if DRAW_SKIA_CLIP

SkPaint DebugClipPaint() {
  SkPaint paint;
  paint.setAntiAlias(true);
  paint.setColor(SK_ColorGREEN);
  paint.setStyle(SkPaint::kStroke_Style);
  return paint;
}

void DebugDrawSkiaClipRect(SkCanvas* canvas, const SkRect& rect) {
  SkPaint paint = DebugClipPaint();
  canvas->drawRect(rect, paint);
}

void DebugDrawSkiaClipPath(SkCanvas* canvas, const SkPath& path) {
  SkPaint paint = DebugClipPaint();
  canvas->drawPath(path, paint);
}

#else  // DRAW_SKIA_CLIP

void DebugDrawSkiaClipRect(SkCanvas* canvas, const SkRect& rect) {}

void DebugDrawSkiaClipPath(SkCanvas* canvas, const SkPath& path) {}

#endif  // DRAW_SKIA_CLIP

#ifdef _SKIA_SUPPORT_
static void DebugValidate(const CFX_RetainPtr<CFX_DIBitmap>& bitmap,
                          const CFX_RetainPtr<CFX_DIBitmap>& device) {
  if (bitmap) {
    SkASSERT(bitmap->GetBPP() == 8 || bitmap->GetBPP() == 32);
    if (bitmap->GetBPP() == 32) {
      bitmap->DebugVerifyBitmapIsPreMultiplied(nullptr);
    }
  }
  if (device) {
    SkASSERT(device->GetBPP() == 8 || device->GetBPP() == 32);
    if (device->GetBPP() == 32) {
      device->DebugVerifyBitmapIsPreMultiplied(nullptr);
    }
  }
}
#endif  // _SKIA_SUPPORT_

SkPath BuildPath(const CFX_PathData* pPathData) {
  SkPath skPath;
  const CFX_PathData* pFPath = pPathData;
  const std::vector<FX_PATHPOINT>& pPoints = pFPath->GetPoints();
  for (size_t i = 0; i < pPoints.size(); i++) {
    CFX_PointF point = pPoints[i].m_Point;
    FXPT_TYPE point_type = pPoints[i].m_Type;
    if (point_type == FXPT_TYPE::MoveTo) {
      skPath.moveTo(point.x, point.y);
    } else if (point_type == FXPT_TYPE::LineTo) {
      skPath.lineTo(point.x, point.y);
    } else if (point_type == FXPT_TYPE::BezierTo) {
      CFX_PointF point2 = pPoints[i + 1].m_Point;
      CFX_PointF point3 = pPoints[i + 2].m_Point;
      skPath.cubicTo(point.x, point.y, point2.x, point2.y, point3.x, point3.y);
      i += 2;
    }
    if (pPoints[i].m_CloseFigure)
      skPath.close();
  }
  return skPath;
}

SkMatrix ToSkMatrix(const CFX_Matrix& m) {
  SkMatrix skMatrix;
  skMatrix.setAll(m.a, m.c, m.e, m.b, m.d, m.f, 0, 0, 1);
  return skMatrix;
}

// use when pdf's y-axis points up instead of down
SkMatrix ToFlippedSkMatrix(const CFX_Matrix& m, SkScalar flip) {
  SkMatrix skMatrix;
  skMatrix.setAll(m.a * flip, -m.c * flip, m.e, m.b * flip, -m.d * flip, m.f, 0,
                  0, 1);
  return skMatrix;
}

SkBlendMode GetSkiaBlendMode(int blend_type) {
  switch (blend_type) {
    case FXDIB_BLEND_MULTIPLY:
      return SkBlendMode::kMultiply;
    case FXDIB_BLEND_SCREEN:
      return SkBlendMode::kScreen;
    case FXDIB_BLEND_OVERLAY:
      return SkBlendMode::kOverlay;
    case FXDIB_BLEND_DARKEN:
      return SkBlendMode::kDarken;
    case FXDIB_BLEND_LIGHTEN:
      return SkBlendMode::kLighten;
    case FXDIB_BLEND_COLORDODGE:
      return SkBlendMode::kColorDodge;
    case FXDIB_BLEND_COLORBURN:
      return SkBlendMode::kColorBurn;
    case FXDIB_BLEND_HARDLIGHT:
      return SkBlendMode::kHardLight;
    case FXDIB_BLEND_SOFTLIGHT:
      return SkBlendMode::kSoftLight;
    case FXDIB_BLEND_DIFFERENCE:
      return SkBlendMode::kDifference;
    case FXDIB_BLEND_EXCLUSION:
      return SkBlendMode::kExclusion;
    case FXDIB_BLEND_HUE:
      return SkBlendMode::kHue;
    case FXDIB_BLEND_SATURATION:
      return SkBlendMode::kSaturation;
    case FXDIB_BLEND_COLOR:
      return SkBlendMode::kColor;
    case FXDIB_BLEND_LUMINOSITY:
      return SkBlendMode::kLuminosity;
    case FXDIB_BLEND_NORMAL:
    default:
      return SkBlendMode::kSrcOver;
  }
}

bool AddColors(const CPDF_ExpIntFunc* pFunc, SkTDArray<SkColor>* skColors) {
  if (pFunc->CountInputs() != 1)
    return false;
  if (pFunc->m_Exponent != 1)
    return false;
  if (pFunc->m_nOrigOutputs != 3)
    return false;
  skColors->push(
      SkColorSetARGB(0xFF, SkUnitScalarClampToByte(pFunc->m_pBeginValues[0]),
                     SkUnitScalarClampToByte(pFunc->m_pBeginValues[1]),
                     SkUnitScalarClampToByte(pFunc->m_pBeginValues[2])));
  skColors->push(
      SkColorSetARGB(0xFF, SkUnitScalarClampToByte(pFunc->m_pEndValues[0]),
                     SkUnitScalarClampToByte(pFunc->m_pEndValues[1]),
                     SkUnitScalarClampToByte(pFunc->m_pEndValues[2])));
  return true;
}

uint8_t FloatToByte(float f) {
  ASSERT(0 <= f && f <= 1);
  return (uint8_t)(f * 255.99f);
}

bool AddSamples(const CPDF_SampledFunc* pFunc,
                SkTDArray<SkColor>* skColors,
                SkTDArray<SkScalar>* skPos) {
  if (pFunc->CountInputs() != 1)
    return false;
  if (pFunc->CountOutputs() != 3)  // expect rgb
    return false;
  if (pFunc->GetEncodeInfo().empty())
    return false;
  const CPDF_SampledFunc::SampleEncodeInfo& encodeInfo =
      pFunc->GetEncodeInfo()[0];
  if (encodeInfo.encode_min != 0)
    return false;
  if (encodeInfo.encode_max != encodeInfo.sizes - 1)
    return false;
  uint32_t sampleSize = pFunc->GetBitsPerSample();
  uint32_t sampleCount = encodeInfo.sizes;
  if (sampleCount != 1U << sampleSize)
    return false;
  if (pFunc->GetSampleStream()->GetSize() < sampleCount * 3 * sampleSize / 8)
    return false;

  float colorsMin[3];
  float colorsMax[3];
  for (int i = 0; i < 3; ++i) {
    colorsMin[i] = pFunc->GetRange(i * 2);
    colorsMax[i] = pFunc->GetRange(i * 2 + 1);
  }
  const uint8_t* pSampleData = pFunc->GetSampleStream()->GetData();
  for (uint32_t i = 0; i < sampleCount; ++i) {
    float floatColors[3];
    for (uint32_t j = 0; j < 3; ++j) {
      int sample = GetBits32(pSampleData, (i * 3 + j) * sampleSize, sampleSize);
      float interp = (float)sample / (sampleCount - 1);
      floatColors[j] = colorsMin[j] + (colorsMax[j] - colorsMin[j]) * interp;
    }
    SkColor color =
        SkPackARGB32(0xFF, FloatToByte(floatColors[0]),
                     FloatToByte(floatColors[1]), FloatToByte(floatColors[2]));
    skColors->push(color);
    skPos->push((float)i / (sampleCount - 1));
  }
  return true;
}

bool AddStitching(const CPDF_StitchFunc* pFunc,
                  SkTDArray<SkColor>* skColors,
                  SkTDArray<SkScalar>* skPos) {
  float boundsStart = pFunc->GetDomain(0);

  const auto& subFunctions = pFunc->GetSubFunctions();
  int subFunctionCount = subFunctions.size();
  for (int i = 0; i < subFunctionCount; ++i) {
    const CPDF_ExpIntFunc* pSubFunc = subFunctions[i]->ToExpIntFunc();
    if (!pSubFunc)
      return false;
    if (!AddColors(pSubFunc, skColors))
      return false;
    float boundsEnd =
        i < subFunctionCount - 1 ? pFunc->GetBound(i + 1) : pFunc->GetDomain(1);
    skPos->push(boundsStart);
    skPos->push(boundsEnd);
    boundsStart = boundsEnd;
  }
  return true;
}

// see https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
SkScalar LineSide(const SkPoint line[2], const SkPoint& pt) {
  return (line[1].fY - line[0].fY) * pt.fX - (line[1].fX - line[0].fX) * pt.fY +
         line[1].fX * line[0].fY - line[1].fY * line[0].fX;
}

SkPoint IntersectSides(const SkPoint& parallelPt,
                       const SkVector& paraRay,
                       const SkPoint& perpendicularPt) {
  SkVector perpRay = {paraRay.fY, -paraRay.fX};
  SkScalar denom = perpRay.fY * paraRay.fX - paraRay.fY * perpRay.fX;
  if (!denom) {
    SkPoint zeroPt = {0, 0};
    return zeroPt;
  }
  SkVector ab0 = parallelPt - perpendicularPt;
  SkScalar numerA = ab0.fY * perpRay.fX - perpRay.fY * ab0.fX;
  numerA /= denom;
  SkPoint result = {parallelPt.fX + paraRay.fX * numerA,
                    parallelPt.fY + paraRay.fY * numerA};
  return result;
}

void ClipAngledGradient(const SkPoint pts[2],
                        SkPoint rectPts[4],
                        bool clipStart,
                        bool clipEnd,
                        SkPath* clip) {
  // find the corners furthest from the gradient perpendiculars
  SkScalar minPerpDist = SK_ScalarMax;
  SkScalar maxPerpDist = SK_ScalarMin;
  int minPerpPtIndex = -1;
  int maxPerpPtIndex = -1;
  SkVector slope = pts[1] - pts[0];
  SkPoint startPerp[2] = {pts[0], {pts[0].fX + slope.fY, pts[0].fY - slope.fX}};
  SkPoint endPerp[2] = {pts[1], {pts[1].fX + slope.fY, pts[1].fY - slope.fX}};
  for (int i = 0; i < 4; ++i) {
    SkScalar sDist = LineSide(startPerp, rectPts[i]);
    SkScalar eDist = LineSide(endPerp, rectPts[i]);
    if (sDist * eDist <= 0)  // if the signs are different,
      continue;              // the point is inside the gradient
    if (sDist < 0) {
      SkScalar smaller = SkTMin(sDist, eDist);
      if (minPerpDist > smaller) {
        minPerpDist = smaller;
        minPerpPtIndex = i;
      }
    } else {
      SkScalar larger = SkTMax(sDist, eDist);
      if (maxPerpDist < larger) {
        maxPerpDist = larger;
        maxPerpPtIndex = i;
      }
    }
  }
  if (minPerpPtIndex < 0 && maxPerpPtIndex < 0)  // nothing's outside
    return;
  // determine if negative distances are before start or after end
  SkPoint beforeStart = {pts[0].fX * 2 - pts[1].fX, pts[0].fY * 2 - pts[1].fY};
  bool beforeNeg = LineSide(startPerp, beforeStart) < 0;
  const SkPoint& startEdgePt =
      clipStart ? pts[0] : beforeNeg ? rectPts[minPerpPtIndex]
                                     : rectPts[maxPerpPtIndex];
  const SkPoint& endEdgePt = clipEnd ? pts[1] : beforeNeg
                                                    ? rectPts[maxPerpPtIndex]
                                                    : rectPts[minPerpPtIndex];
  // find the corners that bound the gradient
  SkScalar minDist = SK_ScalarMax;
  SkScalar maxDist = SK_ScalarMin;
  int minBounds = -1;
  int maxBounds = -1;
  for (int i = 0; i < 4; ++i) {
    SkScalar dist = LineSide(pts, rectPts[i]);
    if (minDist > dist) {
      minDist = dist;
      minBounds = i;
    }
    if (maxDist < dist) {
      maxDist = dist;
      maxBounds = i;
    }
  }
  if (minBounds < 0 || maxBounds < 0)
    return;
  if (minBounds == maxBounds)
    return;
  // construct a clip parallel to the gradient that goes through
  // rectPts[minBounds] and rectPts[maxBounds] and perpendicular to the
  // gradient that goes through startEdgePt, endEdgePt.
  clip->moveTo(IntersectSides(rectPts[minBounds], slope, startEdgePt));
  clip->lineTo(IntersectSides(rectPts[minBounds], slope, endEdgePt));
  clip->lineTo(IntersectSides(rectPts[maxBounds], slope, endEdgePt));
  clip->lineTo(IntersectSides(rectPts[maxBounds], slope, startEdgePt));
}

#ifdef _SKIA_SUPPORT_
void SetBitmapMatrix(const CFX_Matrix* pMatrix,
                     int width,
                     int height,
                     SkMatrix* skMatrix) {
  const CFX_Matrix& m = *pMatrix;
  skMatrix->setAll(m.a / width, -m.c / height, m.c + m.e, m.b / width,
                   -m.d / height, m.d + m.f, 0, 0, 1);
}

void SetBitmapPaint(bool isAlphaMask,
                    uint32_t argb,
                    int bitmap_alpha,
                    int blend_type,
                    SkPaint* paint) {
  paint->setAntiAlias(true);
  if (isAlphaMask) {
    paint->setColorFilter(
        SkColorFilter::MakeModeFilter(argb, SkBlendMode::kSrc));
  }
  // paint->setFilterQuality(kHigh_SkFilterQuality);
  paint->setBlendMode(GetSkiaBlendMode(blend_type));
  paint->setAlpha(bitmap_alpha);
}

bool Upsample(const CFX_RetainPtr<CFX_DIBSource>& pSource,
              std::unique_ptr<uint8_t, FxFreeDeleter>& dst8Storage,
              std::unique_ptr<uint32_t, FxFreeDeleter>& dst32Storage,
              SkBitmap* skBitmap,
              int* widthPtr,
              int* heightPtr,
              bool forceAlpha) {
  void* buffer = pSource->GetBuffer();
  if (!buffer)
    return false;
  SkColorType colorType = forceAlpha || pSource->IsAlphaMask()
                              ? SkColorType::kAlpha_8_SkColorType
                              : SkColorType::kGray_8_SkColorType;
  SkAlphaType alphaType =
      pSource->IsAlphaMask() ? kPremul_SkAlphaType : kOpaque_SkAlphaType;
  int width = pSource->GetWidth();
  int height = pSource->GetHeight();
  int rowBytes = pSource->GetPitch();
  switch (pSource->GetBPP()) {
    case 1: {
      dst8Storage.reset(FX_Alloc2D(uint8_t, width, height));
      uint8_t* dst8Pixels = dst8Storage.get();
      for (int y = 0; y < height; ++y) {
        const uint8_t* srcRow =
            static_cast<const uint8_t*>(buffer) + y * rowBytes;
        uint8_t* dstRow = dst8Pixels + y * width;
        for (int x = 0; x < width; ++x)
          dstRow[x] = srcRow[x >> 3] & (1 << (~x & 0x07)) ? 0xFF : 0x00;
      }
      buffer = dst8Storage.get();
      rowBytes = width;
      break;
    }
    case 8:
      if (pSource->GetPalette()) {
        *ctPtr =
            new SkColorTable(pSource->GetPalette(), pSource->GetPaletteSize());
        colorType = SkColorType::kIndex_8_SkColorType;
      }
      break;
    case 24: {
      dst32Storage.reset(FX_Alloc2D(uint32_t, width, height));
      uint32_t* dst32Pixels = dst32Storage.get();
      for (int y = 0; y < height; ++y) {
        const uint8_t* srcRow =
            static_cast<const uint8_t*>(buffer) + y * rowBytes;
        uint32_t* dstRow = dst32Pixels + y * width;
        for (int x = 0; x < width; ++x) {
          dstRow[x] = SkPackARGB32(0xFF, srcRow[x * 3 + 2], srcRow[x * 3 + 1],
                                   srcRow[x * 3 + 0]);
        }
      }
      buffer = dst32Storage.get();
      rowBytes = width * sizeof(uint32_t);
      colorType = SkColorType::kN32_SkColorType;
      alphaType = kOpaque_SkAlphaType;
      break;
    }
    case 32:
      colorType = SkColorType::kN32_SkColorType;
      alphaType = kPremul_SkAlphaType;
      pSource->DebugVerifyBitmapIsPreMultiplied(buffer);
      break;
    default:
      SkASSERT(0);  // TODO(caryclark) ensure that all cases are covered
      colorType = SkColorType::kUnknown_SkColorType;
  }
  SkImageInfo imageInfo =
      SkImageInfo::Make(width, height, colorType, alphaType);
  skBitmap->installPixels(imageInfo, buffer, rowBytes);
  *widthPtr = width;
  *heightPtr = height;
  return true;
}
#endif  // _SKIA_SUPPORT_

}  // namespace

// Encapsulate the state used for successive text and path draws so that
// they can be combined.
class SkiaState {
 public:
  enum class Clip {
    kSave,
    kPath,
  };

  enum class Accumulator {
    kNone,
    kPath,
    kText,
    kOther,
  };

  // mark all cached state as uninitialized
  explicit SkiaState(CFX_SkiaDeviceDriver* pDriver)
      : m_pDriver(pDriver),
        m_pTypeFace(nullptr),
        m_fontSize(0),
        m_scaleX(0),
        m_fillColor(0),
        m_strokeColor(0),
        m_blendType(0),
        m_commandIndex(0),
        m_drawIndex(INT_MAX),
        m_clipIndex(0),
        m_type(Accumulator::kNone),
        m_fillFullCover(false),
        m_fillPath(false),
        m_groupKnockout(false),
        m_debugDisable(false)
#if SHOW_SKIA_PATH
        ,
        m_debugSaveCounter(0)
#endif
  {
  }

  bool DrawPath(const CFX_PathData* pPathData,
                const CFX_Matrix* pMatrix,
                const CFX_GraphStateData* pDrawState,
                uint32_t fill_color,
                uint32_t stroke_color,
                int fill_mode,
                int blend_type) {
    if (m_debugDisable)
      return false;
    Dump(__func__);
    int drawIndex = SkTMin(m_drawIndex, m_commands.count());
    if (Accumulator::kText == m_type || drawIndex != m_commandIndex ||
        (Accumulator::kPath == m_type &&
         DrawChanged(pMatrix, pDrawState, fill_color, stroke_color, fill_mode,
                     blend_type, m_pDriver->m_bGroupKnockout))) {
      Flush();
    }
    if (Accumulator::kPath != m_type) {
      m_skPath.reset();
      m_fillFullCover = !!(fill_mode & FXFILL_FULLCOVER);
      m_fillPath = (fill_mode & 3) && fill_color;
      m_skPath.setFillType((fill_mode & 3) == FXFILL_ALTERNATE
                               ? SkPath::kEvenOdd_FillType
                               : SkPath::kWinding_FillType);
      if (pDrawState)
        m_drawState.Copy(*pDrawState);
      m_fillColor = fill_color;
      m_strokeColor = stroke_color;
      m_blendType = blend_type;
      m_groupKnockout = m_pDriver->m_bGroupKnockout;
      if (pMatrix)
        m_drawMatrix = *pMatrix;
      m_drawIndex = m_commandIndex;
      m_type = Accumulator::kPath;
    }
    SkPath skPath = BuildPath(pPathData);
    SkPoint delta;
    if (MatrixOffset(pMatrix, &delta))
      skPath.offset(delta.fX, delta.fY);
    m_skPath.addPath(skPath);
    return true;
  }

  void FlushPath() {
    Dump(__func__);
    SkMatrix skMatrix = ToSkMatrix(m_drawMatrix);
    SkPaint skPaint;
    skPaint.setAntiAlias(true);
    if (m_fillFullCover)
      skPaint.setBlendMode(SkBlendMode::kPlus);
    int stroke_alpha = FXARGB_A(m_strokeColor);
    if (stroke_alpha)
      m_pDriver->PaintStroke(&skPaint, &m_drawState, skMatrix);
    SkCanvas* skCanvas = m_pDriver->SkiaCanvas();
    skCanvas->save();
    skCanvas->concat(skMatrix);
    if (m_fillPath) {
      SkPath strokePath;
      const SkPath* fillPath = &m_skPath;
      if (stroke_alpha) {
        if (m_groupKnockout) {
          skPaint.getFillPath(m_skPath, &strokePath);
          if (Op(m_skPath, strokePath, SkPathOp::kDifference_SkPathOp,
                 &strokePath)) {
            fillPath = &strokePath;
          }
        }
      }
      skPaint.setStyle(SkPaint::kFill_Style);
      skPaint.setColor(m_fillColor);
#ifdef _SKIA_SUPPORT_PATHS_
      m_pDriver->PreMultiply();
#endif  // _SKIA_SUPPORT_PATHS_
      DebugShowSkiaDrawPath(m_pDriver.Get(), skCanvas, skPaint, *fillPath);
      skCanvas->drawPath(*fillPath, skPaint);
    }
    if (stroke_alpha) {
      skPaint.setStyle(SkPaint::kStroke_Style);
      skPaint.setColor(m_strokeColor);
#ifdef _SKIA_SUPPORT_PATHS_
      m_pDriver->PreMultiply();
#endif  // _SKIA_SUPPORT_PATHS_
      DebugShowSkiaDrawPath(m_pDriver.Get(), skCanvas, skPaint, m_skPath);
      skCanvas->drawPath(m_skPath, skPaint);
    }
    skCanvas->restore();
    m_drawIndex = INT_MAX;
    m_type = Accumulator::kNone;
  }

  bool HasRSX(int nChars,
              const FXTEXT_CHARPOS* pCharPos,
              float* scaleXPtr,
              bool* oneAtATimePtr) {
    bool useRSXform = false;
    bool oneAtATime = false;
    float scaleX = 1;
    for (int index = 0; index < nChars; ++index) {
      const FXTEXT_CHARPOS& cp = pCharPos[index];
      if (!cp.m_bGlyphAdjust)
        continue;
      bool upright = 0 == cp.m_AdjustMatrix[1] && 0 == cp.m_AdjustMatrix[2];
      if (cp.m_AdjustMatrix[0] != cp.m_AdjustMatrix[3]) {
        if (upright && 1 == cp.m_AdjustMatrix[3]) {
          if (1 == scaleX)
            scaleX = cp.m_AdjustMatrix[0];
          else if (scaleX != cp.m_AdjustMatrix[0])
            oneAtATime = true;
        } else {
          oneAtATime = true;
        }
      } else if (cp.m_AdjustMatrix[1] != -cp.m_AdjustMatrix[2]) {
        oneAtATime = true;
      } else {
        useRSXform = true;
      }
    }
    *oneAtATimePtr = oneAtATime;
    *scaleXPtr = oneAtATime ? 1 : scaleX;
    return oneAtATime ? false : useRSXform;
  }

  bool DrawText(int nChars,
                const FXTEXT_CHARPOS* pCharPos,
                CFX_Font* pFont,
                const CFX_Matrix* pMatrix,
                float font_size,
                uint32_t color) {
    if (m_debugDisable)
      return false;
    Dump(__func__);
    float scaleX = 1;
    bool oneAtATime = false;
    bool hasRSX = HasRSX(nChars, pCharPos, &scaleX, &oneAtATime);
    if (oneAtATime) {
      Flush();
      return false;
    }
    int drawIndex = SkTMin(m_drawIndex, m_commands.count());
    if (Accumulator::kPath == m_type || drawIndex != m_commandIndex ||
        (Accumulator::kText == m_type &&
         (FontChanged(pFont, pMatrix, font_size, scaleX, color) ||
          hasRSX == !m_rsxform.count()))) {
      Flush();
    }
    if (Accumulator::kText != m_type) {
      m_positions.setCount(0);
      m_glyphs.setCount(0);
      m_pTypeFace = pFont->GetFace() ? pFont->GetDeviceCache() : nullptr;
      m_fontSize = font_size;
      m_scaleX = scaleX;
      m_fillColor = color;
      m_drawMatrix = *pMatrix;
      m_drawIndex = m_commandIndex;
      m_type = Accumulator::kText;
    }
    int count = m_positions.count();
    m_positions.setCount(nChars + count);
    m_glyphs.setCount(nChars + count);
    if (hasRSX) {
      m_rsxform.setCount(nChars + count);
    }
    SkScalar flip = m_fontSize < 0 ? -1 : 1;
    SkScalar vFlip = flip;
    if (pFont->IsVertical())
      vFlip *= -1;
    for (int index = 0; index < nChars; ++index) {
      const FXTEXT_CHARPOS& cp = pCharPos[index];
      m_positions[index + count] = {cp.m_Origin.x * flip,
                                    cp.m_Origin.y * vFlip};
      m_glyphs[index + count] = static_cast<uint16_t>(cp.m_GlyphIndex);
    }
    SkPoint delta;
    if (MatrixOffset(pMatrix, &delta)) {
      for (int index = 0; index < nChars; ++index)
        m_positions[index + count].offset(delta.fX * flip, -delta.fY * flip);
    }
    if (hasRSX) {
      for (int index = 0; index < nChars; ++index) {
        const FXTEXT_CHARPOS& cp = pCharPos[index];
        SkRSXform* rsxform = &m_rsxform[index + count];
        if (cp.m_bGlyphAdjust) {
          rsxform->fSCos = cp.m_AdjustMatrix[0];
          rsxform->fSSin = cp.m_AdjustMatrix[1];
          rsxform->fTx = cp.m_AdjustMatrix[0] * m_positions[index].fX;
          rsxform->fTy = cp.m_AdjustMatrix[1] * m_positions[index].fY;
        } else {
          rsxform->fSCos = 1;
          rsxform->fSSin = 0;
          rsxform->fTx = m_positions[index].fX;
          rsxform->fTy = m_positions[index].fY;
        }
      }
    }
    return true;
  }

  void FlushText() {
    Dump(__func__);
    SkPaint skPaint;
    skPaint.setAntiAlias(true);
    skPaint.setColor(m_fillColor);
    if (m_pTypeFace) {  // exclude placeholder test fonts
      sk_sp<SkTypeface> typeface(SkSafeRef(m_pTypeFace.Get()));
      skPaint.setTypeface(typeface);
    }
    skPaint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    skPaint.setHinting(SkPaint::kNo_Hinting);
    skPaint.setTextScaleX(m_scaleX);
    skPaint.setTextSize(SkTAbs(m_fontSize));
    skPaint.setSubpixelText(true);
    SkCanvas* skCanvas = m_pDriver->SkiaCanvas();
    skCanvas->save();
    SkScalar flip = m_fontSize < 0 ? -1 : 1;
    SkMatrix skMatrix = ToFlippedSkMatrix(m_drawMatrix, flip);
    skCanvas->concat(skMatrix);
#ifdef _SKIA_SUPPORT_PATHS_
    m_pDriver->PreMultiply();
#endif  // _SKIA_SUPPORT_PATHS_
#if SHOW_TEXT_GLYPHS
    SkTDArray<SkUnichar> text;
    text.setCount(m_glyphs.count());
    skPaint.glyphsToUnichars(m_glyphs.begin(), m_glyphs.count(), text.begin());
    for (size_t i = 0; i < m_glyphs.count(); ++i)
      printf("%lc", m_glyphs[i]);
    printf("\n");
#endif
    if (m_rsxform.count()) {
      skCanvas->drawTextRSXform(m_glyphs.begin(), m_glyphs.count() * 2,
                                m_rsxform.begin(), nullptr, skPaint);
    } else {
      skCanvas->drawPosText(m_glyphs.begin(), m_glyphs.count() * 2,
                            m_positions.begin(), skPaint);
    }
    skCanvas->restore();
    m_drawIndex = INT_MAX;
    m_type = Accumulator::kNone;
  }

  bool IsEmpty() { return !m_commands.count(); }

  bool SetClipFill(const CFX_PathData* pPathData,
                   const CFX_Matrix* pMatrix,
                   int fill_mode) {
    if (m_debugDisable)
      return false;
    Dump(__func__);
    SkPath skClipPath;
    if (pPathData->GetPoints().size() == 5 ||
        pPathData->GetPoints().size() == 4) {
      CFX_FloatRect rectf;
      if (pPathData->IsRect(pMatrix, &rectf)) {
        rectf.Intersect(CFX_FloatRect(
            0, 0,
            static_cast<float>(m_pDriver->GetDeviceCaps(FXDC_PIXEL_WIDTH)),
            static_cast<float>(m_pDriver->GetDeviceCaps(FXDC_PIXEL_HEIGHT))));
        FX_RECT outer = rectf.GetOuterRect();
        // note that PDF's y-axis goes up; Skia's y-axis goes down
        skClipPath.addRect({(float)outer.left, (float)outer.bottom,
                            (float)outer.right, (float)outer.top});
      }
    }
    if (skClipPath.isEmpty()) {
      skClipPath = BuildPath(pPathData);
      skClipPath.setFillType((fill_mode & 3) == FXFILL_ALTERNATE
                                 ? SkPath::kEvenOdd_FillType
                                 : SkPath::kWinding_FillType);
      SkMatrix skMatrix = ToSkMatrix(*pMatrix);
      skClipPath.transform(skMatrix);
    }
    return SetClip(skClipPath);
  }

  bool SetClip(const SkPath& skClipPath) {
    // if a pending draw depends on clip state that is cached, flush it and draw
    if (m_commandIndex < m_commands.count()) {
      if (m_commands[m_commandIndex] == Clip::kPath &&
          m_clips[m_commandIndex] == skClipPath) {
        ++m_commandIndex;
        return true;
      }
      Flush();
    }
    while (m_clipIndex > m_commandIndex) {
      do {
        --m_clipIndex;
        SkASSERT(m_clipIndex >= 0);
      } while (m_commands[m_clipIndex] != Clip::kSave);
      m_pDriver->SkiaCanvas()->restore();
    }
    if (m_commandIndex < m_commands.count()) {
      m_commands[m_commandIndex] = Clip::kPath;
      m_clips[m_commandIndex] = skClipPath;
    } else {
      m_commands.push(Clip::kPath);
      m_clips.push_back(skClipPath);
    }
    ++m_commandIndex;
    return true;
  }

  bool SetClipStroke(const CFX_PathData* pPathData,
                     const CFX_Matrix* pMatrix,
                     const CFX_GraphStateData* pGraphState) {
    if (m_debugDisable)
      return false;
    Dump(__func__);
    SkPath skPath = BuildPath(pPathData);
    SkMatrix skMatrix = ToSkMatrix(*pMatrix);
    SkPaint skPaint;
    m_pDriver->PaintStroke(&skPaint, pGraphState, skMatrix);
    SkPath dst_path;
    skPaint.getFillPath(skPath, &dst_path);
    dst_path.transform(skMatrix);
    return SetClip(dst_path);
  }

  bool MatrixOffset(const CFX_Matrix* pMatrix, SkPoint* delta) {
    CFX_Matrix identityMatrix;
    if (!pMatrix)
      pMatrix = &identityMatrix;
    delta->set(pMatrix->e - m_drawMatrix.e, pMatrix->f - m_drawMatrix.f);
    if (!delta->fX && !delta->fY)
      return true;
    SkMatrix drawMatrix = ToSkMatrix(m_drawMatrix);
    if (!(drawMatrix.getType() & ~SkMatrix::kTranslate_Mask))
      return true;
    SkMatrix invDrawMatrix;
    if (!drawMatrix.invert(&invDrawMatrix))
      return false;
    SkMatrix invNewMatrix;
    SkMatrix newMatrix = ToSkMatrix(*pMatrix);
    if (!newMatrix.invert(&invNewMatrix))
      return false;
    delta->set(invDrawMatrix.getTranslateX() - invNewMatrix.getTranslateX(),
               invDrawMatrix.getTranslateY() - invNewMatrix.getTranslateY());
    return true;
  }

  // returns true if caller should apply command to skia canvas
  bool ClipSave() {
    if (m_debugDisable)
      return false;
    Dump(__func__);
    int count = m_commands.count();
    if (m_commandIndex < count) {
      if (Clip::kSave == m_commands[m_commandIndex]) {
        ++m_commandIndex;
        return true;
      }
      Flush();
      AdjustClip(m_commandIndex);
      m_commands[m_commandIndex] = Clip::kSave;
      m_clips[m_commandIndex] = m_skEmptyPath;
    } else {
      AdjustClip(m_commandIndex);
      m_commands.push(Clip::kSave);
      m_clips.push_back(m_skEmptyPath);
    }
    ++m_commandIndex;
    return true;
  }

  bool ClipRestore() {
    if (m_debugDisable)
      return false;
    Dump(__func__);
    while (Clip::kSave != m_commands[--m_commandIndex]) {
      SkASSERT(m_commandIndex > 0);
    }
    return true;
  }

  bool DrawChanged(const CFX_Matrix* pMatrix,
                   const CFX_GraphStateData* pState,
                   uint32_t fill_color,
                   uint32_t stroke_color,
                   int fill_mode,
                   int blend_type,
                   bool group_knockout) const {
    return MatrixChanged(pMatrix, m_drawMatrix) ||
           StateChanged(pState, m_drawState) || fill_color != m_fillColor ||
           stroke_color != m_strokeColor ||
           ((fill_mode & 3) == FXFILL_ALTERNATE) !=
               (m_skPath.getFillType() == SkPath::kEvenOdd_FillType) ||
           blend_type != m_blendType || group_knockout != m_groupKnockout;
  }

  bool FontChanged(CFX_Font* pFont,
                   const CFX_Matrix* pMatrix,
                   float font_size,
                   float scaleX,
                   uint32_t color) const {
    CFX_TypeFace* typeface =
        pFont->GetFace() ? pFont->GetDeviceCache() : nullptr;
    return typeface != m_pTypeFace || MatrixChanged(pMatrix, m_drawMatrix) ||
           font_size != m_fontSize || scaleX != m_scaleX ||
           color != m_fillColor;
  }

  bool MatrixChanged(const CFX_Matrix* pMatrix,
                     const CFX_Matrix& refMatrix) const {
    CFX_Matrix identityMatrix;
    if (!pMatrix)
      pMatrix = &identityMatrix;
    return pMatrix->a != refMatrix.a || pMatrix->b != refMatrix.b ||
           pMatrix->c != refMatrix.c || pMatrix->d != refMatrix.d;
  }

  bool StateChanged(const CFX_GraphStateData* pState,
                    const CFX_GraphStateData& refState) const {
    CFX_GraphStateData identityState;
    if (!pState)
      pState = &identityState;
    return pState->m_LineWidth != refState.m_LineWidth ||
           pState->m_LineCap != refState.m_LineCap ||
           pState->m_LineJoin != refState.m_LineJoin ||
           pState->m_MiterLimit != refState.m_MiterLimit ||
           DashChanged(pState, refState);
  }

  bool DashChanged(const CFX_GraphStateData* pState,
                   const CFX_GraphStateData& refState) const {
    bool dashArray = pState && pState->m_DashArray;
    if (!dashArray && !refState.m_DashArray)
      return false;
    if (!dashArray || !refState.m_DashArray)
      return true;
    if (pState->m_DashPhase != refState.m_DashPhase ||
        pState->m_DashCount != refState.m_DashCount) {
      return true;
    }
    for (int index = 0; index < pState->m_DashCount; ++index) {
      if (pState->m_DashArray[index] != refState.m_DashArray[index])
        return true;
    }
    return true;
  }

  void AdjustClip(int limit) {
    while (m_clipIndex > limit) {
      do {
        --m_clipIndex;
        SkASSERT(m_clipIndex >= 0);
      } while (m_commands[m_clipIndex] != Clip::kSave);
      m_pDriver->SkiaCanvas()->restore();
    }
    while (m_clipIndex < limit) {
      if (Clip::kSave == m_commands[m_clipIndex]) {
        m_pDriver->SkiaCanvas()->save();
      } else {
        SkASSERT(Clip::kPath == m_commands[m_clipIndex]);
        m_pDriver->SkiaCanvas()->clipPath(m_clips[m_clipIndex],
                                          SkClipOp::kIntersect, true);
      }
      ++m_clipIndex;
    }
  }

  void Flush() {
    if (m_debugDisable)
      return;
    Dump(__func__);
    if (Accumulator::kPath == m_type || Accumulator::kText == m_type) {
      AdjustClip(SkTMin(m_drawIndex, m_commands.count()));
      Accumulator::kPath == m_type ? FlushPath() : FlushText();
    }
  }

  void FlushForDraw() {
    if (m_debugDisable)
      return;
    Flush();                     // draw any pending text or path
    AdjustClip(m_commandIndex);  // set up clip stack with any pending state
  }

#if SHOW_SKIA_PATH
  void DumpPrefix(int index) const {
    if (index != m_commandIndex && index != m_drawIndex &&
        index != m_clipIndex) {
      printf("     ");
      return;
    }
    printf("%c%c%c> ", index == m_commandIndex ? 'x' : '-',
           index == m_drawIndex ? 'd' : '-', index == m_clipIndex ? 'c' : '-');
  }

  void DumpEndPrefix() const {
    int index = m_commands.count();
    if (index != m_commandIndex && index > m_drawIndex && index != m_clipIndex)
      return;
    printf("%c%c%c>\n", index == m_commandIndex ? 'x' : '-',
           index <= m_drawIndex ? 'd' : '-', index == m_clipIndex ? 'c' : '-');
  }
#endif  // SHOW_SKIA_PATH

  void Dump(const char* where) const {
#if SHOW_SKIA_PATH
    if (m_debugDisable)
      return;
    printf(
        "\n%s\nSkia Save Count %d  Agg Save Stack/Count %d/%d"
        "  Cache Save Index/Count %d/%d\n",
        where, m_pDriver->m_pCanvas->getSaveCount(),
        (int)m_pDriver->m_StateStack.size(), AggSaveCount(m_pDriver),
        m_commandIndex, CacheSaveCount(m_commands, m_commandIndex));
    printf("Cache:\n");
#if SHOW_SKIA_PATH_SHORTHAND
    bool dumpedPath = false;
#endif
    for (int index = 0; index < m_commands.count(); ++index) {
#if SHOW_SKIA_PATH_SHORTHAND
      if (Clip::kSave == m_commands[index] && dumpedPath) {
        printf("\n");
        dumpedPath = false;
      }
#endif
      DumpPrefix(index);
      switch (m_commands[index]) {
        case Clip::kSave:
          printf("Save %d\n", ++m_debugSaveCounter);
          break;
        case Clip::kPath:
#if SHOW_SKIA_PATH_SHORTHAND
          printf("*");
          dumpedPath = true;
#else
          m_clips[index].dump();
#endif
          break;
        default:
          printf("unknown\n");
      }
    }
#if SHOW_SKIA_PATH_SHORTHAND
    if (dumpedPath)
      printf("\n");
#endif
    DumpEndPrefix();
    int skCanvasSaveCount = m_pDriver->m_pCanvas->getSaveCount();
    int cacheSaveCount = 1;
    SkASSERT(m_clipIndex <= m_commands.count());
    for (int index = 0; index < m_clipIndex; ++index)
      cacheSaveCount += Clip::kSave == m_commands[index];
    SkASSERT(skCanvasSaveCount == cacheSaveCount);
#endif  // SHOW_SKIA_PATH
  }

#if SHOW_SKIA_PATH
  static int AggSaveCount(const CFX_UnownedPtr<CFX_SkiaDeviceDriver> driver) {
    FX_RECT last;
    int aggSaveCount = 0;
    bool foundLast = false;
    for (int index = 0; index < (int)driver->m_StateStack.size(); ++index) {
      if (!driver->m_StateStack[index]) {
        continue;
      }
      if (driver->m_StateStack[index]->GetType() != CFX_ClipRgn::RectI) {
        aggSaveCount += 1;
        foundLast = false;
        continue;
      }
      if (!foundLast || memcmp(&last, &driver->m_StateStack[index]->GetBox(),
                               sizeof(FX_RECT))) {
        aggSaveCount += 1;
        foundLast = true;
        last = driver->m_StateStack[index]->GetBox();
      }
    }
    if (driver->m_pClipRgn) {
      CFX_ClipRgn::ClipType clipType = driver->m_pClipRgn->GetType();
      if (clipType != CFX_ClipRgn::RectI || !foundLast ||
          memcmp(&last, &driver->m_pClipRgn->GetBox(), sizeof(FX_RECT))) {
        aggSaveCount += 1;
      }
    }
    return aggSaveCount;
  }

  static int CacheSaveCount(const SkTDArray<SkiaState::Clip>& commands,
                            int commandIndex) {
    int cacheSaveCount = 0;
    bool newPath = false;
    for (int index = 0; index < commandIndex; ++index) {
      if (Clip::kSave == commands[index]) {
        newPath = true;
      } else if (newPath) {
        ++cacheSaveCount;
        newPath = false;
      }
    }
    return cacheSaveCount;
  }
#endif

  void DebugCheckClip() {
#if SHOW_SKIA_PATH
    if (m_debugDisable)
      return;
    int aggSaveCount = AggSaveCount(m_pDriver);
    int cacheSaveCount = CacheSaveCount(m_commands, m_commandIndex);
    SkASSERT(m_clipIndex <= m_commands.count());
    if (aggSaveCount != cacheSaveCount) {
      // may not signify a bug if counts don't match
      printf("aggSaveCount %d != cacheSaveCount %d\n", aggSaveCount,
             cacheSaveCount);
      DumpClipStacks();
    }
    for (int aggIndex = 0; aggIndex < (int)m_pDriver->m_StateStack.size();
         ++aggIndex) {
      if (!m_pDriver->m_StateStack[aggIndex])
        continue;
      if (m_pDriver->m_StateStack[aggIndex]->GetType() != CFX_ClipRgn::RectI)
        continue;
      const FX_RECT& aggRect = m_pDriver->m_StateStack[aggIndex]->GetBox();
      SkRect skRect = SkRect::MakeLTRB(aggRect.left, aggRect.top, aggRect.right,
                                       aggRect.bottom);
      bool foundMatch = false;
      for (int skIndex = 0; skIndex < m_commandIndex; ++skIndex) {
        if (Clip::kPath != m_commands[skIndex])
          continue;
        const SkPath& clip = m_clips[skIndex];
        SkRect bounds;
        if (!clip.isRect(&bounds))
          continue;
        bounds.roundOut(&bounds);
        if (skRect == bounds) {
          foundMatch = true;
          break;
        }
      }
      if (!foundMatch) {
        DumpClipStacks();
        SkASSERT(0);
      }
    }
#endif  // SHOW_SKIA_PATH
  }

#if SHOW_SKIA_PATH
  void DumpClipStacks() const {
    if (m_debugDisable)
      return;
    printf("\ncache\n");
    for (int index = 0; index < m_commandIndex; ++index) {
      DumpPrefix(index);
      switch (m_commands[index]) {
        case Clip::kSave:
          printf("Save\n");
          break;
        case Clip::kPath:
          m_clips[index].dump();
          break;
        default:
          printf("unknown\n");
      }
    }
    printf("\nagg\n");
    for (int index = 0; index < (int)m_pDriver->m_StateStack.size(); ++index) {
      if (!m_pDriver->m_StateStack[index]) {
        printf("null\n");
        continue;
      }
      CFX_ClipRgn::ClipType clipType =
          m_pDriver->m_StateStack[index]->GetType();
      const FX_RECT& box = m_pDriver->m_StateStack[index]->GetBox();
      printf("stack rect: %d,%d,%d,%d mask=%s\n", box.left, box.top, box.right,
             box.bottom,
             CFX_ClipRgn::MaskF == clipType
                 ? "1"
                 : CFX_ClipRgn::RectI == clipType ? "0" : "?");
    }
    if (m_pDriver->m_pClipRgn) {
      const FX_RECT& box = m_pDriver->m_pClipRgn->GetBox();
      CFX_ClipRgn::ClipType clipType = m_pDriver->m_pClipRgn->GetType();
      printf("clip rect: %d,%d,%d,%d mask=%s\n", box.left, box.top, box.right,
             box.bottom,
             CFX_ClipRgn::MaskF == clipType
                 ? "1"
                 : CFX_ClipRgn::RectI == clipType ? "0" : "?");
    }
  }
#endif  // SHOW_SKIA_PATH

 private:
  SkTArray<SkPath> m_clips;        // stack of clips that may be reused
  SkTDArray<Clip> m_commands;      // stack of clip-related commands
  SkTDArray<SkPoint> m_positions;  // accumulator for text positions
  SkTDArray<SkRSXform> m_rsxform;  // accumulator for txt rotate/scale/translate
  SkTDArray<uint16_t> m_glyphs;    // accumulator for text glyphs
  SkPath m_skPath;                 // accumulator for path contours
  SkPath m_skEmptyPath;            // used as placehold in the clips array
  CFX_Matrix m_drawMatrix;
  CFX_GraphStateData m_clipState;
  CFX_GraphStateData m_drawState;
  CFX_Matrix m_clipMatrix;
  CFX_UnownedPtr<CFX_SkiaDeviceDriver> m_pDriver;
  CFX_UnownedPtr<CFX_TypeFace> m_pTypeFace;
  float m_fontSize;
  float m_scaleX;
  uint32_t m_fillColor;
  uint32_t m_strokeColor;
  int m_blendType;
  int m_commandIndex;  // active position in clip command stack
  int m_drawIndex;     // position of the pending path or text draw
  int m_clipIndex;     // position reflecting depth of canvas clip stacck
  Accumulator m_type;  // type of pending draw
  bool m_fillFullCover;
  bool m_fillPath;
  bool m_groupKnockout;
  bool m_debugDisable;  // turn off cache for debugging
#if SHOW_SKIA_PATH
 public:
  mutable int m_debugSaveCounter;
  static int m_debugInitCounter;
#endif
};

#if SHOW_SKIA_PATH
int SkiaState::m_debugInitCounter;
#endif

// convert a stroking path to scanlines
void CFX_SkiaDeviceDriver::PaintStroke(SkPaint* spaint,
                                       const CFX_GraphStateData* pGraphState,
                                       const SkMatrix& matrix) {
  SkPaint::Cap cap;
  switch (pGraphState->m_LineCap) {
    case CFX_GraphStateData::LineCapRound:
      cap = SkPaint::kRound_Cap;
      break;
    case CFX_GraphStateData::LineCapSquare:
      cap = SkPaint::kSquare_Cap;
      break;
    default:
      cap = SkPaint::kButt_Cap;
      break;
  }
  SkPaint::Join join;
  switch (pGraphState->m_LineJoin) {
    case CFX_GraphStateData::LineJoinRound:
      join = SkPaint::kRound_Join;
      break;
    case CFX_GraphStateData::LineJoinBevel:
      join = SkPaint::kBevel_Join;
      break;
    default:
      join = SkPaint::kMiter_Join;
      break;
  }
  SkMatrix inverse;
  if (!matrix.invert(&inverse))
    return;  // give up if the matrix is degenerate, and not invertable
  inverse.set(SkMatrix::kMTransX, 0);
  inverse.set(SkMatrix::kMTransY, 0);
  SkVector deviceUnits[2] = {{0, 1}, {1, 0}};
  inverse.mapPoints(deviceUnits, SK_ARRAY_COUNT(deviceUnits));
  float width =
      SkTMax(pGraphState->m_LineWidth,
             SkTMin(deviceUnits[0].length(), deviceUnits[1].length()));
  if (pGraphState->m_DashArray) {
    int count = (pGraphState->m_DashCount + 1) / 2;
    std::unique_ptr<SkScalar, FxFreeDeleter> intervals(
        FX_Alloc2D(SkScalar, count, sizeof(SkScalar)));
    // Set dash pattern
    for (int i = 0; i < count; i++) {
      float on = pGraphState->m_DashArray[i * 2];
      if (on <= 0.000001f)
        on = 1.f / 10;
      float off = i * 2 + 1 == pGraphState->m_DashCount
                      ? on
                      : pGraphState->m_DashArray[i * 2 + 1];
      if (off < 0)
        off = 0;
      intervals.get()[i * 2] = on;
      intervals.get()[i * 2 + 1] = off;
    }
    spaint->setPathEffect(SkDashPathEffect::Make(intervals.get(), count * 2,
                                                 pGraphState->m_DashPhase));
  }
  spaint->setStyle(SkPaint::kStroke_Style);
  spaint->setAntiAlias(true);
  spaint->setStrokeWidth(width);
  spaint->setStrokeMiter(pGraphState->m_MiterLimit);
  spaint->setStrokeCap(cap);
  spaint->setStrokeJoin(join);
}

CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(
    const CFX_RetainPtr<CFX_DIBitmap>& pBitmap,
    bool bRgbByteOrder,
    const CFX_RetainPtr<CFX_DIBitmap>& pOriDevice,
    bool bGroupKnockout)
    : m_pBitmap(pBitmap),
      m_pOriDevice(pOriDevice),
      m_pRecorder(nullptr),
      m_pCache(new SkiaState(this)),
#ifdef _SKIA_SUPPORT_PATHS_
      m_pClipRgn(nullptr),
      m_FillFlags(0),
      m_bRgbByteOrder(bRgbByteOrder),
#endif  // _SKIA_SUPPORT_PATHS_
      m_bGroupKnockout(bGroupKnockout) {
  SkBitmap skBitmap;
  SkASSERT(pBitmap->GetBPP() == 8 || pBitmap->GetBPP() == 32);
  SkImageInfo imageInfo = SkImageInfo::Make(
      pBitmap->GetWidth(), pBitmap->GetHeight(),
      pBitmap->GetBPP() == 8 ? kAlpha_8_SkColorType : kN32_SkColorType,
      kOpaque_SkAlphaType);
  skBitmap.installPixels(imageInfo, pBitmap->GetBuffer(), pBitmap->GetPitch());
  m_pCanvas = new SkCanvas(skBitmap);
}

#ifdef _SKIA_SUPPORT_
CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(int size_x, int size_y)
    : m_pBitmap(nullptr),
      m_pOriDevice(nullptr),
      m_pRecorder(new SkPictureRecorder),
      m_pCache(new SkiaState(this)),
      m_bGroupKnockout(false) {
  m_pRecorder->beginRecording(SkIntToScalar(size_x), SkIntToScalar(size_y));
  m_pCanvas = m_pRecorder->getRecordingCanvas();
}

CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(SkPictureRecorder* recorder)
    : m_pBitmap(nullptr),
      m_pOriDevice(nullptr),
      m_pRecorder(recorder),
      m_pCache(new SkiaState(this)),
      m_bGroupKnockout(false) {
  m_pCanvas = m_pRecorder->getRecordingCanvas();
}
#endif  // _SKIA_SUPPORT_

CFX_SkiaDeviceDriver::~CFX_SkiaDeviceDriver() {
  Flush();
  if (!m_pRecorder)
    delete m_pCanvas;
}

void CFX_SkiaDeviceDriver::Flush() {
  m_pCache->Flush();
}

bool CFX_SkiaDeviceDriver::DrawDeviceText(int nChars,
                                          const FXTEXT_CHARPOS* pCharPos,
                                          CFX_Font* pFont,
                                          const CFX_Matrix* pObject2Device,
                                          float font_size,
                                          uint32_t color) {
  if (m_pCache->DrawText(nChars, pCharPos, pFont, pObject2Device, font_size,
                         color)) {
    return true;
  }
  sk_sp<SkTypeface> typeface(SkSafeRef(pFont->GetDeviceCache()));
  SkPaint paint;
  paint.setAntiAlias(true);
  paint.setColor(color);
  paint.setTypeface(typeface);
  paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
  paint.setHinting(SkPaint::kNo_Hinting);
  paint.setTextSize(SkTAbs(font_size));
  paint.setSubpixelText(true);
  m_pCanvas->save();
  SkScalar flip = font_size < 0 ? -1 : 1;
  SkScalar vFlip = flip;
  if (pFont->IsVertical())
    vFlip *= -1;
  SkMatrix skMatrix = ToFlippedSkMatrix(*pObject2Device, flip);
  m_pCanvas->concat(skMatrix);
  SkTDArray<SkPoint> positions;
  positions.setCount(nChars);
  SkTDArray<uint16_t> glyphs;
  glyphs.setCount(nChars);
  bool useRSXform = false;
  bool oneAtATime = false;
  for (int index = 0; index < nChars; ++index) {
    const FXTEXT_CHARPOS& cp = pCharPos[index];
    positions[index] = {cp.m_Origin.x * flip, cp.m_Origin.y * vFlip};
    if (cp.m_bGlyphAdjust) {
      useRSXform = true;
      if (cp.m_AdjustMatrix[0] != cp.m_AdjustMatrix[3] ||
          cp.m_AdjustMatrix[1] != -cp.m_AdjustMatrix[2]) {
        oneAtATime = true;
      }
    }
    glyphs[index] = static_cast<uint16_t>(cp.m_GlyphIndex);
  }
  if (oneAtATime)
    useRSXform = false;
#if SHOW_TEXT_GLYPHS
  SkTDArray<SkUnichar> text;
  text.setCount(glyphs.count());
  paint.glyphsToUnichars(glyphs.begin(), glyphs.count(), text.begin());
  for (size_t i = 0; i < glyphs.count(); ++i)
    printf("%lc", text[i]);
  printf("\n");
#endif
#ifdef _SKIA_SUPPORT_PATHS_
  m_pBitmap->PreMultiply();
#endif  // _SKIA_SUPPORT_PATHS_
  if (useRSXform) {
    SkTDArray<SkRSXform> xforms;
    xforms.setCount(nChars);
    for (int index = 0; index < nChars; ++index) {
      const FXTEXT_CHARPOS& cp = pCharPos[index];
      SkRSXform* rsxform = &xforms[index];
      if (cp.m_bGlyphAdjust) {
        rsxform->fSCos = cp.m_AdjustMatrix[0];
        rsxform->fSSin = cp.m_AdjustMatrix[1];
        rsxform->fTx = cp.m_AdjustMatrix[0] * positions[index].fX;
        rsxform->fTy = cp.m_AdjustMatrix[1] * positions[index].fY;
      } else {
        rsxform->fSCos = 1;
        rsxform->fSSin = 0;
        rsxform->fTx = positions[index].fX;
        rsxform->fTy = positions[index].fY;
      }
    }
    m_pCanvas->drawTextRSXform(glyphs.begin(), nChars * 2, xforms.begin(),
                               nullptr, paint);
  } else if (oneAtATime) {
    for (int index = 0; index < nChars; ++index) {
      const FXTEXT_CHARPOS& cp = pCharPos[index];
      if (cp.m_bGlyphAdjust) {
        if (0 == cp.m_AdjustMatrix[1] && 0 == cp.m_AdjustMatrix[2] &&
            1 == cp.m_AdjustMatrix[3]) {
          paint.setTextScaleX(cp.m_AdjustMatrix[0]);
          m_pCanvas->drawText(&glyphs[index], 1, positions[index].fX,
                              positions[index].fY, paint);
          paint.setTextScaleX(1);
        } else {
          m_pCanvas->save();
          SkMatrix adjust;
          adjust.reset();
          adjust.setScaleX(cp.m_AdjustMatrix[0]);
          adjust.setSkewX(cp.m_AdjustMatrix[1]);
          adjust.setSkewY(cp.m_AdjustMatrix[2]);
          adjust.setScaleY(cp.m_AdjustMatrix[3]);
          adjust.preTranslate(positions[index].fX, positions[index].fY);
          m_pCanvas->concat(adjust);
          m_pCanvas->drawText(&glyphs[index], 1, 0, 0, paint);
          m_pCanvas->restore();
        }
      } else {
        m_pCanvas->drawText(&glyphs[index], 1, positions[index].fX,
                            positions[index].fY, paint);
      }
    }
  } else {
    m_pCanvas->drawPosText(glyphs.begin(), nChars * 2, positions.begin(),
                           paint);
  }
  m_pCanvas->restore();

  return true;
}

int CFX_SkiaDeviceDriver::GetDeviceCaps(int caps_id) const {
  switch (caps_id) {
    case FXDC_DEVICE_CLASS:
      return FXDC_DISPLAY;
#ifdef _SKIA_SUPPORT_
    case FXDC_PIXEL_WIDTH:
      return m_pCanvas->imageInfo().width();
    case FXDC_PIXEL_HEIGHT:
      return m_pCanvas->imageInfo().height();
    case FXDC_BITS_PIXEL:
      return 32;
    case FXDC_HORZ_SIZE:
    case FXDC_VERT_SIZE:
      return 0;
    case FXDC_RENDER_CAPS:
      return FXRC_GET_BITS | FXRC_ALPHA_PATH | FXRC_ALPHA_IMAGE |
             FXRC_BLEND_MODE | FXRC_SOFT_CLIP | FXRC_ALPHA_OUTPUT |
             FXRC_FILLSTROKE_PATH | FXRC_SHADING;
#endif  // _SKIA_SUPPORT_

#ifdef _SKIA_SUPPORT_PATHS_
    case FXDC_PIXEL_WIDTH:
      return m_pBitmap->GetWidth();
    case FXDC_PIXEL_HEIGHT:
      return m_pBitmap->GetHeight();
    case FXDC_BITS_PIXEL:
      return m_pBitmap->GetBPP();
    case FXDC_HORZ_SIZE:
    case FXDC_VERT_SIZE:
      return 0;
    case FXDC_RENDER_CAPS: {
      int flags = FXRC_GET_BITS | FXRC_ALPHA_PATH | FXRC_ALPHA_IMAGE |
                  FXRC_BLEND_MODE | FXRC_SOFT_CLIP | FXRC_SHADING;
      if (m_pBitmap->HasAlpha()) {
        flags |= FXRC_ALPHA_OUTPUT;
      } else if (m_pBitmap->IsAlphaMask()) {
        if (m_pBitmap->GetBPP() == 1) {
          flags |= FXRC_BITMASK_OUTPUT;
        } else {
          flags |= FXRC_BYTEMASK_OUTPUT;
        }
      }
      if (m_pBitmap->IsCmykImage()) {
        flags |= FXRC_CMYK_OUTPUT;
      }
      return flags;
    }
#endif  // _SKIA_SUPPORT_PATHS_
  }
  return 0;
}

void CFX_SkiaDeviceDriver::SaveState() {
  m_pCache->DebugCheckClip();
  if (!m_pCache->ClipSave())
    m_pCanvas->save();

#ifdef _SKIA_SUPPORT_PATHS_
#if SHOW_SKIA_PATH
  printf("SaveState %zd\n", m_StateStack.size());
#endif
  std::unique_ptr<CFX_ClipRgn> pClip;
  if (m_pClipRgn)
    pClip = pdfium::MakeUnique<CFX_ClipRgn>(*m_pClipRgn);
  m_StateStack.push_back(std::move(pClip));
#endif  // _SKIA_SUPPORT_PATHS_
}

void CFX_SkiaDeviceDriver::RestoreState(bool bKeepSaved) {
#ifdef _SKIA_SUPPORT_PATHS_
  m_pClipRgn.reset();

  if (m_StateStack.empty())
    return;
#else
  if (m_pCache->IsEmpty())
    return;
#endif
  if (!m_pCache->ClipRestore())
    m_pCanvas->restore();
  if (bKeepSaved && !m_pCache->ClipSave())
    m_pCanvas->save();
#ifdef _SKIA_SUPPORT_PATHS_
#if SHOW_SKIA_PATH
  printf("RestoreState %zd %s\n", m_StateStack.size(),
         bKeepSaved ? "bKeepSaved" : "");
#endif
  if (bKeepSaved) {
    if (m_StateStack.back())
      m_pClipRgn = pdfium::MakeUnique<CFX_ClipRgn>(*m_StateStack.back());
  } else {
    m_pClipRgn = std::move(m_StateStack.back());
    m_StateStack.pop_back();
  }
  m_pCache->DebugCheckClip();
#endif  // _SKIA_SUPPORT_PATHS_
}

#ifdef _SKIA_SUPPORT_PATHS_
void CFX_SkiaDeviceDriver::SetClipMask(const FX_RECT& clipBox,
                                       const SkPath& path) {
  FX_RECT path_rect(clipBox.left, clipBox.top, clipBox.right + 1,
                    clipBox.bottom + 1);
  path_rect.Intersect(m_pClipRgn->GetBox());
  auto pThisLayer = pdfium::MakeRetain<CFX_DIBitmap>();
  pThisLayer->Create(path_rect.Width(), path_rect.Height(), FXDIB_8bppMask);
  pThisLayer->Clear(0);

  SkImageInfo imageInfo =
      SkImageInfo::Make(pThisLayer->GetWidth(), pThisLayer->GetHeight(),
                        SkColorType::kAlpha_8_SkColorType, kOpaque_SkAlphaType);
  SkBitmap bitmap;
  bitmap.installPixels(imageInfo, pThisLayer->GetBuffer(),
                       pThisLayer->GetPitch());
  auto canvas = pdfium::MakeUnique<SkCanvas>(bitmap);
  canvas->translate(
      -path_rect.left,
      -path_rect.top);  // FIXME(caryclark) wrong sign(s)? upside down?
  SkPaint paint;
  paint.setAntiAlias((m_FillFlags & FXFILL_NOPATHSMOOTH) == 0);
  canvas->drawPath(path, paint);
  m_pClipRgn->IntersectMaskF(path_rect.left, path_rect.top, pThisLayer);
}
#endif  // _SKIA_SUPPORT_PATHS_

bool CFX_SkiaDeviceDriver::SetClip_PathFill(
    const CFX_PathData* pPathData,     // path info
    const CFX_Matrix* pObject2Device,  // flips object's y-axis
    int fill_mode                      // fill mode, WINDING or ALTERNATE
    ) {
  CFX_Matrix identity;
  const CFX_Matrix* deviceMatrix = pObject2Device ? pObject2Device : &identity;
  bool cached = m_pCache->SetClipFill(pPathData, deviceMatrix, fill_mode);

#ifdef _SKIA_SUPPORT_PATHS_
  m_FillFlags = fill_mode;
  if (!m_pClipRgn) {
    m_pClipRgn = pdfium::MakeUnique<CFX_ClipRgn>(
        GetDeviceCaps(FXDC_PIXEL_WIDTH), GetDeviceCaps(FXDC_PIXEL_HEIGHT));
  }
#endif  // _SKIA_SUPPORT_PATHS_
  if (pPathData->GetPoints().size() == 5 ||
      pPathData->GetPoints().size() == 4) {
    CFX_FloatRect rectf;
    if (pPathData->IsRect(deviceMatrix, &rectf)) {
      rectf.Intersect(CFX_FloatRect(0, 0,
                                    (float)GetDeviceCaps(FXDC_PIXEL_WIDTH),
                                    (float)GetDeviceCaps(FXDC_PIXEL_HEIGHT)));
      // note that PDF's y-axis goes up; Skia's y-axis goes down
      if (!cached) {
        SkRect skClipRect =
            SkRect::MakeLTRB(rectf.left, rectf.bottom, rectf.right, rectf.top);
        DebugDrawSkiaClipRect(m_pCanvas, skClipRect);
        m_pCanvas->clipRect(skClipRect, SkClipOp::kIntersect, true);
      }

#ifdef _SKIA_SUPPORT_PATHS_
      FX_RECT rect = rectf.GetOuterRect();
      m_pClipRgn->IntersectRect(rect);
#endif  // _SKIA_SUPPORT_PATHS_
      DebugShowCanvasClip(this, m_pCanvas);
      return true;
    }
  }
  SkPath skClipPath = BuildPath(pPathData);
  skClipPath.setFillType((fill_mode & 3) == FXFILL_ALTERNATE
                             ? SkPath::kEvenOdd_FillType
                             : SkPath::kWinding_FillType);
  SkMatrix skMatrix = ToSkMatrix(*deviceMatrix);
  skClipPath.transform(skMatrix);
  DebugShowSkiaPath(skClipPath);
  if (!cached) {
    DebugDrawSkiaClipPath(m_pCanvas, skClipPath);
    m_pCanvas->clipPath(skClipPath, SkClipOp::kIntersect, true);
  }
#ifdef _SKIA_SUPPORT_PATHS_
  FX_RECT clipBox(0, 0, GetDeviceCaps(FXDC_PIXEL_WIDTH),
                  GetDeviceCaps(FXDC_PIXEL_HEIGHT));
  SetClipMask(clipBox, skClipPath);
#endif  // _SKIA_SUPPORT_PATHS_
  DebugShowCanvasClip(this, m_pCanvas);
  return true;
}

bool CFX_SkiaDeviceDriver::SetClip_PathStroke(
    const CFX_PathData* pPathData,         // path info
    const CFX_Matrix* pObject2Device,      // optional transformation
    const CFX_GraphStateData* pGraphState  // graphic state, for pen attributes
    ) {
  bool cached = m_pCache->SetClipStroke(pPathData, pObject2Device, pGraphState);

#ifdef _SKIA_SUPPORT_PATHS_
  if (!m_pClipRgn) {
    m_pClipRgn = pdfium::MakeUnique<CFX_ClipRgn>(
        GetDeviceCaps(FXDC_PIXEL_WIDTH), GetDeviceCaps(FXDC_PIXEL_HEIGHT));
  }
#endif  // _SKIA_SUPPORT_PATHS_
  // build path data
  SkPath skPath = BuildPath(pPathData);
  SkMatrix skMatrix = ToSkMatrix(*pObject2Device);
  SkPaint skPaint;
  PaintStroke(&skPaint, pGraphState, skMatrix);
  SkPath dst_path;
  skPaint.getFillPath(skPath, &dst_path);
  dst_path.transform(skMatrix);
  if (!cached) {
    DebugDrawSkiaClipPath(m_pCanvas, dst_path);
    m_pCanvas->clipPath(dst_path, SkClipOp::kIntersect, true);
  }
#ifdef _SKIA_SUPPORT_PATHS_
  FX_RECT clipBox(0, 0, GetDeviceCaps(FXDC_PIXEL_WIDTH),
                  GetDeviceCaps(FXDC_PIXEL_HEIGHT));
  SetClipMask(clipBox, dst_path);
#endif  // _SKIA_SUPPORT_PATHS_
  DebugShowCanvasClip(this, m_pCanvas);
  return true;
}

bool CFX_SkiaDeviceDriver::DrawPath(
    const CFX_PathData* pPathData,          // path info
    const CFX_Matrix* pObject2Device,       // optional transformation
    const CFX_GraphStateData* pGraphState,  // graphic state, for pen attributes
    uint32_t fill_color,                    // fill color
    uint32_t stroke_color,                  // stroke color
    int fill_mode,  // fill mode, WINDING or ALTERNATE. 0 for not filled
    int blend_type) {
  if (fill_mode & FX_ZEROAREA_FILL)
    return true;
  if (m_pCache->DrawPath(pPathData, pObject2Device, pGraphState, fill_color,
                         stroke_color, fill_mode, blend_type)) {
    return true;
  }
  SkMatrix skMatrix;
  if (pObject2Device)
    skMatrix = ToSkMatrix(*pObject2Device);
  else
    skMatrix.setIdentity();
  SkPaint skPaint;
  skPaint.setAntiAlias(true);
  if (fill_mode & FXFILL_FULLCOVER)
    skPaint.setBlendMode(SkBlendMode::kPlus);
  int stroke_alpha = FXARGB_A(stroke_color);
  if (pGraphState && stroke_alpha)
    PaintStroke(&skPaint, pGraphState, skMatrix);
  SkPath skPath = BuildPath(pPathData);
  m_pCanvas->save();
  m_pCanvas->concat(skMatrix);
  if ((fill_mode & 3) && fill_color) {
    skPath.setFillType((fill_mode & 3) == FXFILL_ALTERNATE
                           ? SkPath::kEvenOdd_FillType
                           : SkPath::kWinding_FillType);
    SkPath strokePath;
    const SkPath* fillPath = &skPath;
    if (pGraphState && stroke_alpha) {
      if (m_bGroupKnockout) {
        skPaint.getFillPath(skPath, &strokePath);
        if (Op(skPath, strokePath, SkPathOp::kDifference_SkPathOp,
               &strokePath)) {
          fillPath = &strokePath;
        }
      }
    }
    skPaint.setStyle(SkPaint::kFill_Style);
    skPaint.setColor(fill_color);
#ifdef _SKIA_SUPPORT_PATHS_
    m_pBitmap->PreMultiply();
#endif  // _SKIA_SUPPORT_PATHS_
    DebugShowSkiaDrawPath(this, m_pCanvas, skPaint, *fillPath);
    m_pCanvas->drawPath(*fillPath, skPaint);
  }
  if (pGraphState && stroke_alpha) {
    skPaint.setStyle(SkPaint::kStroke_Style);
    skPaint.setColor(stroke_color);
#ifdef _SKIA_SUPPORT_PATHS_
    m_pBitmap->PreMultiply();
#endif  // _SKIA_SUPPORT_PATHS_
    DebugShowSkiaDrawPath(this, m_pCanvas, skPaint, skPath);
    m_pCanvas->drawPath(skPath, skPaint);
  }
  m_pCanvas->restore();
  return true;
}

bool CFX_SkiaDeviceDriver::DrawCosmeticLine(float x1,
                                            float y1,
                                            float x2,
                                            float y2,
                                            uint32_t color,
                                            int blend_type) {
  return false;
}

bool CFX_SkiaDeviceDriver::FillRectWithBlend(const FX_RECT* pRect,
                                             uint32_t fill_color,
                                             int blend_type) {
  m_pCache->FlushForDraw();
  SkPaint spaint;
  spaint.setAntiAlias(true);
  spaint.setColor(fill_color);
  spaint.setBlendMode(GetSkiaBlendMode(blend_type));
  SkRect rect =
      SkRect::MakeLTRB(pRect->left, SkTMin(pRect->top, pRect->bottom),
                       pRect->right, SkTMax(pRect->bottom, pRect->top));
  DebugShowSkiaDrawRect(this, m_pCanvas, spaint, rect);
  m_pCanvas->drawRect(rect, spaint);
  return true;
}

bool CFX_SkiaDeviceDriver::DrawShading(const CPDF_ShadingPattern* pPattern,
                                       const CFX_Matrix* pMatrix,
                                       const FX_RECT& clip_rect,
                                       int alpha,
                                       bool bAlphaMode) {
  m_pCache->FlushForDraw();
  ShadingType shadingType = pPattern->GetShadingType();
  if (kAxialShading != shadingType && kRadialShading != shadingType &&
      kCoonsPatchMeshShading != shadingType) {
    // TODO(caryclark) more types
    return false;
  }
  int csFamily = pPattern->GetCS()->GetFamily();
  if (PDFCS_DEVICERGB != csFamily && PDFCS_DEVICEGRAY != csFamily)
    return false;
  const std::vector<std::unique_ptr<CPDF_Function>>& pFuncs =
      pPattern->GetFuncs();
  int nFuncs = pFuncs.size();
  if (nFuncs > 1)  // TODO(caryclark) remove this restriction
    return false;
  CPDF_Dictionary* pDict = pPattern->GetShadingObject()->GetDict();
  CPDF_Array* pCoords = pDict->GetArrayFor("Coords");
  if (!pCoords && kCoonsPatchMeshShading != shadingType)
    return false;
  // TODO(caryclark) Respect Domain[0], Domain[1]. (Don't know what they do
  // yet.)
  SkTDArray<SkColor> skColors;
  SkTDArray<SkScalar> skPos;
  for (int j = 0; j < nFuncs; j++) {
    if (!pFuncs[j])
      continue;

    if (const CPDF_SampledFunc* pSampledFunc = pFuncs[j]->ToSampledFunc()) {
      /* TODO(caryclark)
         Type 0 Sampled Functions in PostScript can also have an Order integer
         in the dictionary. PDFium doesn't appear to check for this anywhere.
       */
      if (!AddSamples(pSampledFunc, &skColors, &skPos))
        return false;
    } else if (const CPDF_ExpIntFunc* pExpIntFuc = pFuncs[j]->ToExpIntFunc()) {
      if (!AddColors(pExpIntFuc, &skColors))
        return false;
      skPos.push(0);
      skPos.push(1);
    } else if (const CPDF_StitchFunc* pStitchFunc = pFuncs[j]->ToStitchFunc()) {
      if (!AddStitching(pStitchFunc, &skColors, &skPos))
        return false;
    } else {
      return false;
    }
  }
  CPDF_Array* pArray = pDict->GetArrayFor("Extend");
  bool clipStart = !pArray || !pArray->GetIntegerAt(0);
  bool clipEnd = !pArray || !pArray->GetIntegerAt(1);
  SkPaint paint;
  paint.setAntiAlias(true);
  paint.setAlpha(alpha);
  SkMatrix skMatrix = ToSkMatrix(*pMatrix);
  SkRect skRect = SkRect::MakeLTRB(clip_rect.left, clip_rect.top,
                                   clip_rect.right, clip_rect.bottom);
  SkPath skClip;
  SkPath skPath;
  if (kAxialShading == shadingType) {
    float start_x = pCoords->GetNumberAt(0);
    float start_y = pCoords->GetNumberAt(1);
    float end_x = pCoords->GetNumberAt(2);
    float end_y = pCoords->GetNumberAt(3);
    SkPoint pts[] = {{start_x, start_y}, {end_x, end_y}};
    skMatrix.mapPoints(pts, SK_ARRAY_COUNT(pts));
    paint.setShader(SkGradientShader::MakeLinear(
        pts, skColors.begin(), skPos.begin(), skColors.count(),
        SkShader::kClamp_TileMode));
    if (clipStart || clipEnd) {
      // if the gradient is horizontal or vertical, modify the draw rectangle
      if (pts[0].fX == pts[1].fX) {  // vertical
        if (pts[0].fY > pts[1].fY) {
          SkTSwap(pts[0].fY, pts[1].fY);
          SkTSwap(clipStart, clipEnd);
        }
        if (clipStart)
          skRect.fTop = SkTMax(skRect.fTop, pts[0].fY);
        if (clipEnd)
          skRect.fBottom = SkTMin(skRect.fBottom, pts[1].fY);
      } else if (pts[0].fY == pts[1].fY) {  // horizontal
        if (pts[0].fX > pts[1].fX) {
          SkTSwap(pts[0].fX, pts[1].fX);
          SkTSwap(clipStart, clipEnd);
        }
        if (clipStart)
          skRect.fLeft = SkTMax(skRect.fLeft, pts[0].fX);
        if (clipEnd)
          skRect.fRight = SkTMin(skRect.fRight, pts[1].fX);
      } else {  // if the gradient is angled and contained by the rect, clip
        SkPoint rectPts[4] = {{skRect.fLeft, skRect.fTop},
                              {skRect.fRight, skRect.fTop},
                              {skRect.fRight, skRect.fBottom},
                              {skRect.fLeft, skRect.fBottom}};
        ClipAngledGradient(pts, rectPts, clipStart, clipEnd, &skClip);
      }
    }
    skPath.addRect(skRect);
    skMatrix.setIdentity();
  } else if (kRadialShading == shadingType) {
    float start_x = pCoords->GetNumberAt(0);
    float start_y = pCoords->GetNumberAt(1);
    float start_r = pCoords->GetNumberAt(2);
    float end_x = pCoords->GetNumberAt(3);
    float end_y = pCoords->GetNumberAt(4);
    float end_r = pCoords->GetNumberAt(5);
    SkPoint pts[] = {{start_x, start_y}, {end_x, end_y}};

    paint.setShader(SkGradientShader::MakeTwoPointConical(
        pts[0], start_r, pts[1], end_r, skColors.begin(), skPos.begin(),
        skColors.count(), SkShader::kClamp_TileMode));
    if (clipStart || clipEnd) {
      if (clipStart && start_r)
        skClip.addCircle(pts[0].fX, pts[0].fY, start_r);
      if (clipEnd)
        skClip.addCircle(pts[1].fX, pts[1].fY, end_r, SkPath::kCCW_Direction);
      else
        skClip.setFillType(SkPath::kInverseWinding_FillType);
      skClip.transform(skMatrix);
    }
    SkMatrix inverse;
    if (!skMatrix.invert(&inverse))
      return false;
    skPath.addRect(skRect);
    skPath.transform(inverse);
  } else {
    ASSERT(kCoonsPatchMeshShading == shadingType);
    CPDF_Stream* pStream = ToStream(pPattern->GetShadingObject());
    if (!pStream)
      return false;
    CPDF_MeshStream stream(shadingType, pPattern->GetFuncs(), pStream,
                           pPattern->GetCS());
    if (!stream.Load())
      return false;
    SkPoint cubics[12];
    SkColor colors[4];
    m_pCanvas->save();
    if (!skClip.isEmpty())
      m_pCanvas->clipPath(skClip, SkClipOp::kIntersect, true);
    m_pCanvas->concat(skMatrix);
    while (!stream.BitStream()->IsEOF()) {
      uint32_t flag = stream.ReadFlag();
      int iStartPoint = flag ? 4 : 0;
      int iStartColor = flag ? 2 : 0;
      if (flag) {
        SkPoint tempCubics[4];
        for (int i = 0; i < (int)SK_ARRAY_COUNT(tempCubics); i++)
          tempCubics[i] = cubics[(flag * 3 + i) % 12];
        memcpy(cubics, tempCubics, sizeof(tempCubics));
        SkColor tempColors[2];
        tempColors[0] = colors[flag];
        tempColors[1] = colors[(flag + 1) % 4];
        memcpy(colors, tempColors, sizeof(tempColors));
      }
      for (int i = iStartPoint; i < (int)SK_ARRAY_COUNT(cubics); i++) {
        CFX_PointF point = stream.ReadCoords();
        cubics[i].fX = point.x;
        cubics[i].fY = point.y;
      }
      for (int i = iStartColor; i < (int)SK_ARRAY_COUNT(colors); i++) {
        float r;
        float g;
        float b;
        std::tie(r, g, b) = stream.ReadColor();
        colors[i] = SkColorSetARGBInline(0xFF, (U8CPU)(r * 255),
                                         (U8CPU)(g * 255), (U8CPU)(b * 255));
      }
      m_pCanvas->drawPatch(cubics, colors, nullptr, paint);
    }
    m_pCanvas->restore();
    return true;
  }
  m_pCanvas->save();
  if (!skClip.isEmpty())
    m_pCanvas->clipPath(skClip, SkClipOp::kIntersect, true);
  m_pCanvas->concat(skMatrix);
  m_pCanvas->drawPath(skPath, paint);
  m_pCanvas->restore();
  return true;
}

uint8_t* CFX_SkiaDeviceDriver::GetBuffer() const {
  return m_pBitmap->GetBuffer();
}

bool CFX_SkiaDeviceDriver::GetClipBox(FX_RECT* pRect) {
#ifdef _SKIA_SUPPORT_PATHS_
  if (!m_pClipRgn) {
    pRect->left = pRect->top = 0;
    pRect->right = GetDeviceCaps(FXDC_PIXEL_WIDTH);
    pRect->bottom = GetDeviceCaps(FXDC_PIXEL_HEIGHT);
    return true;
  }
  *pRect = m_pClipRgn->GetBox();
#else
  // TODO(caryclark) call m_canvas->getClipDeviceBounds() instead
  pRect->left = 0;
  pRect->top = 0;
  const SkImageInfo& canvasSize = m_pCanvas->imageInfo();
  pRect->right = canvasSize.width();
  pRect->bottom = canvasSize.height();
#endif
  return true;
}

bool CFX_SkiaDeviceDriver::GetDIBits(const CFX_RetainPtr<CFX_DIBitmap>& pBitmap,
                                     int left,
                                     int top) {
  if (!m_pBitmap)
    return true;
  uint8_t* srcBuffer = m_pBitmap->GetBuffer();
  if (!srcBuffer)
    return true;
#ifdef _SKIA_SUPPORT_
  m_pCache->FlushForDraw();
  int srcWidth = m_pBitmap->GetWidth();
  int srcHeight = m_pBitmap->GetHeight();
  int srcRowBytes = srcWidth * sizeof(uint32_t);
  SkImageInfo srcImageInfo = SkImageInfo::Make(
      srcWidth, srcHeight, SkColorType::kN32_SkColorType, kPremul_SkAlphaType);
  SkBitmap skSrcBitmap;
  skSrcBitmap.installPixels(srcImageInfo, srcBuffer, srcRowBytes);
  SkASSERT(pBitmap);
  uint8_t* dstBuffer = pBitmap->GetBuffer();
  SkASSERT(dstBuffer);
  int dstWidth = pBitmap->GetWidth();
  int dstHeight = pBitmap->GetHeight();
  int dstRowBytes = dstWidth * sizeof(uint32_t);
  SkImageInfo dstImageInfo = SkImageInfo::Make(
      dstWidth, dstHeight, SkColorType::kN32_SkColorType, kPremul_SkAlphaType);
  SkBitmap skDstBitmap;
  skDstBitmap.installPixels(dstImageInfo, dstBuffer, dstRowBytes);
  SkCanvas canvas(skDstBitmap);
  canvas.drawBitmap(skSrcBitmap, left, top, nullptr);
  return true;
#endif  // _SKIA_SUPPORT_

#ifdef _SKIA_SUPPORT_PATHS_
  Flush();
  m_pBitmap->UnPreMultiply();
  FX_RECT rect(left, top, left + pBitmap->GetWidth(),
               top + pBitmap->GetHeight());
  CFX_RetainPtr<CFX_DIBitmap> pBack;
  if (m_pOriDevice) {
    pBack = m_pOriDevice->Clone(&rect);
    if (!pBack)
      return true;

    pBack->CompositeBitmap(0, 0, pBack->GetWidth(), pBack->GetHeight(),
                           m_pBitmap, 0, 0);
  } else {
    pBack = m_pBitmap->Clone(&rect);
    if (!pBack)
      return true;
  }

  bool bRet = true;
  left = std::min(left, 0);
  top = std::min(top, 0);
  if (m_bRgbByteOrder) {
    RgbByteOrderTransferBitmap(pBitmap, 0, 0, rect.Width(), rect.Height(),
                               pBack, left, top);
  } else {
    bRet = pBitmap->TransferBitmap(0, 0, rect.Width(), rect.Height(), pBack,
                                   left, top);
  }
  return bRet;
#endif  // _SKIA_SUPPORT_PATHS_
}

CFX_RetainPtr<CFX_DIBitmap> CFX_SkiaDeviceDriver::GetBackDrop() {
  return m_pOriDevice;
}

bool CFX_SkiaDeviceDriver::SetDIBits(
    const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
    uint32_t argb,
    const FX_RECT* pSrcRect,
    int left,
    int top,
    int blend_type) {
  if (!m_pBitmap || !m_pBitmap->GetBuffer())
    return true;

#ifdef _SKIA_SUPPORT_
  CFX_Matrix m(pBitmap->GetWidth(), 0, 0, -pBitmap->GetHeight(), left,
               top + pBitmap->GetHeight());
  std::unique_ptr<CFX_ImageRenderer> dummy;
  return StartDIBits(pBitmap, 0xFF, argb, &m, 0, &dummy, blend_type);
#endif  // _SKIA_SUPPORT_

#ifdef _SKIA_SUPPORT_PATHS_
  Flush();
  if (pBitmap->IsAlphaMask()) {
    return m_pBitmap->CompositeMask(left, top, pSrcRect->Width(),
                                    pSrcRect->Height(), pBitmap, argb,
                                    pSrcRect->left, pSrcRect->top, blend_type,
                                    m_pClipRgn.get(), m_bRgbByteOrder, 0);
  }
  return m_pBitmap->CompositeBitmap(
      left, top, pSrcRect->Width(), pSrcRect->Height(), pBitmap, pSrcRect->left,
      pSrcRect->top, blend_type, m_pClipRgn.get(), m_bRgbByteOrder);
#endif  // _SKIA_SUPPORT_PATHS_
}

bool CFX_SkiaDeviceDriver::StretchDIBits(
    const CFX_RetainPtr<CFX_DIBSource>& pSource,
    uint32_t argb,
    int dest_left,
    int dest_top,
    int dest_width,
    int dest_height,
    const FX_RECT* pClipRect,
    uint32_t flags,
    int blend_type) {
#ifdef _SKIA_SUPPORT_
  m_pCache->FlushForDraw();
  if (!m_pBitmap->GetBuffer())
    return true;
  CFX_Matrix m(dest_width, 0, 0, -dest_height, dest_left,
               dest_top + dest_height);

  m_pCanvas->save();
  SkRect skClipRect = SkRect::MakeLTRB(pClipRect->left, pClipRect->bottom,
                                       pClipRect->right, pClipRect->top);
  m_pCanvas->clipRect(skClipRect, SkClipOp::kIntersect, true);
  std::unique_ptr<CFX_ImageRenderer> dummy;
  bool result = StartDIBits(pSource, 0xFF, argb, &m, 0, &dummy, blend_type);
  m_pCanvas->restore();

  return result;
#endif  // _SKIA_SUPPORT_

#ifdef _SKIA_SUPPORT_PATHS_
  if (dest_width == pSource->GetWidth() &&
      dest_height == pSource->GetHeight()) {
    FX_RECT rect(0, 0, dest_width, dest_height);
    return SetDIBits(pSource, argb, &rect, dest_left, dest_top, blend_type);
  }
  Flush();
  FX_RECT dest_rect(dest_left, dest_top, dest_left + dest_width,
                    dest_top + dest_height);
  dest_rect.Normalize();
  FX_RECT dest_clip = dest_rect;
  dest_clip.Intersect(*pClipRect);
  CFX_BitmapComposer composer;
  composer.Compose(m_pBitmap, m_pClipRgn.get(), 255, argb, dest_clip, false,
                   false, false, m_bRgbByteOrder, 0, blend_type);
  dest_clip.Offset(-dest_rect.left, -dest_rect.top);
  CFX_ImageStretcher stretcher(&composer, pSource, dest_width, dest_height,
                               dest_clip, flags);
  if (stretcher.Start())
    stretcher.Continue(nullptr);
  return true;
#endif  // _SKIA_SUPPORT_PATHS_
}

bool CFX_SkiaDeviceDriver::StartDIBits(
    const CFX_RetainPtr<CFX_DIBSource>& pSource,
    int bitmap_alpha,
    uint32_t argb,
    const CFX_Matrix* pMatrix,
    uint32_t render_flags,
    std::unique_ptr<CFX_ImageRenderer>* handle,
    int blend_type) {
#ifdef _SKIA_SUPPORT_
  m_pCache->FlushForDraw();
  DebugValidate(m_pBitmap, m_pOriDevice);
  std::unique_ptr<uint8_t, FxFreeDeleter> dst8Storage;
  std::unique_ptr<uint32_t, FxFreeDeleter> dst32Storage;
  SkBitmap skBitmap;
  int width, height;
  if (!Upsample(pSource, dst8Storage, dst32Storage, &skBitmap, &width, &height,
                false)) {
    return false;
  }
  m_pCanvas->save();
  SkMatrix skMatrix;
  SetBitmapMatrix(pMatrix, width, height, &skMatrix);
  m_pCanvas->concat(skMatrix);
  SkPaint paint;
  SetBitmapPaint(pSource->IsAlphaMask(), argb, bitmap_alpha, blend_type,
                 &paint);
  // TODO(caryclark) Once Skia supports 8 bit src to 8 bit dst remove this
  if (m_pBitmap && m_pBitmap->GetBPP() == 8 && pSource->GetBPP() == 8) {
    SkMatrix inv;
    SkAssertResult(skMatrix.invert(&inv));
    for (int y = 0; y < m_pBitmap->GetHeight(); ++y) {
      for (int x = 0; x < m_pBitmap->GetWidth(); ++x) {
        SkPoint src = {x + 0.5f, y + 0.5f};
        inv.mapPoints(&src, 1);
        // TODO(caryclark) Why does the matrix map require clamping?
        src.fX = SkTMax(0.5f, SkTMin(src.fX, width - 0.5f));
        src.fY = SkTMax(0.5f, SkTMin(src.fY, height - 0.5f));
        m_pBitmap->SetPixel(x, y, skBitmap.getColor(src.fX, src.fY));
      }
    }
  } else {
    m_pCanvas->drawBitmap(skBitmap, 0, 0, &paint);
  }
  m_pCanvas->restore();
  DebugValidate(m_pBitmap, m_pOriDevice);
#endif  // _SKIA_SUPPORT_

#ifdef _SKIA_SUPPORT_PATHS_
  Flush();
  if (!m_pBitmap->GetBuffer())
    return true;
  m_pBitmap->UnPreMultiply();
  *handle = pdfium::MakeUnique<CFX_ImageRenderer>(
      m_pBitmap, m_pClipRgn.get(), pSource, bitmap_alpha, argb, pMatrix,
      render_flags, m_bRgbByteOrder);
#endif  // _SKIA_SUPPORT_PATHS_
  return true;
}

bool CFX_SkiaDeviceDriver::ContinueDIBits(CFX_ImageRenderer* handle,
                                          IFX_Pause* pPause) {
#ifdef _SKIA_SUPPORT_
  m_pCache->FlushForDraw();
  return false;
#endif  // _SKIA_SUPPORT_

#ifdef _SKIA_SUPPORT_PATHS_
  Flush();
  if (!m_pBitmap->GetBuffer()) {
    return true;
  }
  return handle->Continue(pPause);
#endif  // _SKIA_SUPPORT_PATHS_
}

#if defined _SKIA_SUPPORT_
void CFX_SkiaDeviceDriver::PreMultiply(
    const CFX_RetainPtr<CFX_DIBitmap>& pDIBitmap) {
  pDIBitmap->PreMultiply();
}
#endif  // _SKIA_SUPPORT_

void CFX_DIBitmap::PreMultiply() {
  if (this->GetBPP() != 32)
    return;
  void* buffer = this->GetBuffer();
  if (!buffer)
    return;
#if defined _SKIA_SUPPORT_PATHS_
  Format priorFormat = m_nFormat;
  m_nFormat = Format::kPreMultiplied;
  if (priorFormat != Format::kUnPreMultiplied)
    return;
#endif
  int height = this->GetHeight();
  int width = this->GetWidth();
  int rowBytes = this->GetPitch();
  SkImageInfo unpremultipliedInfo =
      SkImageInfo::Make(width, height, kN32_SkColorType, kUnpremul_SkAlphaType);
  SkPixmap unpremultiplied(unpremultipliedInfo, buffer, rowBytes);
  SkImageInfo premultipliedInfo =
      SkImageInfo::Make(width, height, kN32_SkColorType, kPremul_SkAlphaType);
  SkPixmap premultiplied(premultipliedInfo, buffer, rowBytes);
  unpremultiplied.readPixels(premultiplied);
  this->DebugVerifyBitmapIsPreMultiplied(nullptr);
}

#ifdef _SKIA_SUPPORT_PATHS_
void CFX_DIBitmap::UnPreMultiply() {
  if (this->GetBPP() != 32)
    return;
  void* buffer = this->GetBuffer();
  if (!buffer)
    return;
  Format priorFormat = m_nFormat;
  m_nFormat = Format::kUnPreMultiplied;
  if (priorFormat != Format::kPreMultiplied)
    return;
  this->DebugVerifyBitmapIsPreMultiplied(nullptr);
  int height = this->GetHeight();
  int width = this->GetWidth();
  int rowBytes = this->GetPitch();
  SkImageInfo premultipliedInfo =
      SkImageInfo::Make(width, height, kN32_SkColorType, kPremul_SkAlphaType);
  SkPixmap premultiplied(premultipliedInfo, buffer, rowBytes);
  SkImageInfo unpremultipliedInfo =
      SkImageInfo::Make(width, height, kN32_SkColorType, kUnpremul_SkAlphaType);
  SkPixmap unpremultiplied(unpremultipliedInfo, buffer, rowBytes);
  premultiplied.readPixels(unpremultiplied);
}
#endif  // _SKIA_SUPPORT_PATHS_

#ifdef _SKIA_SUPPORT_
bool CFX_SkiaDeviceDriver::DrawBitsWithMask(
    const CFX_RetainPtr<CFX_DIBSource>& pSource,
    const CFX_RetainPtr<CFX_DIBSource>& pMask,
    int bitmap_alpha,
    const CFX_Matrix* pMatrix,
    int blend_type) {
  DebugValidate(m_pBitmap, m_pOriDevice);
  std::unique_ptr<uint8_t, FxFreeDeleter> src8Storage, mask8Storage;
  std::unique_ptr<uint32_t, FxFreeDeleter> src32Storage, mask32Storage;
  SkBitmap skBitmap, skMask;
  int srcWidth, srcHeight, maskWidth, maskHeight;
  if (!Upsample(pSource, src8Storage, src32Storage, &skBitmap, &srcWidth,
                &srcHeight, false)) {
    return false;
  }
  if (!Upsample(pMask, mask8Storage, mask32Storage, &skMask, &maskWidth,
                &maskHeight, true)) {
    return false;
  }
  m_pCanvas->save();
  SkMatrix skMatrix;
  SetBitmapMatrix(pMatrix, srcWidth, srcHeight, &skMatrix);
  m_pCanvas->concat(skMatrix);
  SkPaint paint;
  SetBitmapPaint(pSource->IsAlphaMask(), 0xFFFFFFFF, bitmap_alpha, blend_type,
                 &paint);
  sk_sp<SkImage> skSrc = SkImage::MakeFromBitmap(skBitmap);
  sk_sp<SkShader> skSrcShader =
      skSrc->makeShader(SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
  sk_sp<SkImage> skMaskImage = SkImage::MakeFromBitmap(skMask);
  sk_sp<SkShader> skMaskShader = skMaskImage->makeShader(
      SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
  paint.setShader(SkShader::MakeComposeShader(skMaskShader, skSrcShader,
                                              SkBlendMode::kSrcIn));
  SkRect r = {0, 0, SkIntToScalar(srcWidth), SkIntToScalar(srcHeight)};
  m_pCanvas->drawRect(r, paint);
  m_pCanvas->restore();
  DebugValidate(m_pBitmap, m_pOriDevice);
  return true;
}

bool CFX_SkiaDeviceDriver::SetBitsWithMask(
    const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
    const CFX_RetainPtr<CFX_DIBSource>& pMask,
    int dest_left,
    int dest_top,
    int bitmap_alpha,
    int blend_type) {
  if (!m_pBitmap || !m_pBitmap->GetBuffer())
    return true;
  CFX_Matrix m(pBitmap->GetWidth(), 0, 0, -pBitmap->GetHeight(), dest_left,
               dest_top + pBitmap->GetHeight());
  return DrawBitsWithMask(pBitmap, pMask, bitmap_alpha, &m, blend_type);
}

void CFX_SkiaDeviceDriver::Clear(uint32_t color) {
  m_pCanvas->clear(color);
}
#endif  // _SKIA_SUPPORT_

void CFX_SkiaDeviceDriver::Dump() const {
#if SHOW_SKIA_PATH && defined _SKIA_SUPPORT_
  if (m_pCache)
    m_pCache->Dump(this);
#endif  // SHOW_SKIA_PATH && defined _SKIA_SUPPORT_
}

#ifdef _SKIA_SUPPORT_
void CFX_SkiaDeviceDriver::DebugVerifyBitmapIsPreMultiplied() const {
  if (m_pOriDevice)
    m_pOriDevice->DebugVerifyBitmapIsPreMultiplied(nullptr);
}
#endif  // _SKIA_SUPPORT_

CFX_DefaultRenderDevice::CFX_DefaultRenderDevice() {}

#ifdef _SKIA_SUPPORT_
void CFX_DefaultRenderDevice::Clear(uint32_t color) {
  CFX_SkiaDeviceDriver* skDriver =
      static_cast<CFX_SkiaDeviceDriver*>(GetDeviceDriver());
  skDriver->Clear(color);
}

SkPictureRecorder* CFX_DefaultRenderDevice::CreateRecorder(int size_x,
                                                           int size_y) {
  CFX_SkiaDeviceDriver* skDriver = new CFX_SkiaDeviceDriver(size_x, size_y);
  SetDeviceDriver(pdfium::WrapUnique(skDriver));
  return skDriver->GetRecorder();
}
#endif  // _SKIA_SUPPORT_

bool CFX_DefaultRenderDevice::Attach(
    const CFX_RetainPtr<CFX_DIBitmap>& pBitmap,
    bool bRgbByteOrder,
    const CFX_RetainPtr<CFX_DIBitmap>& pOriDevice,
    bool bGroupKnockout) {
  if (!pBitmap)
    return false;
  SetBitmap(pBitmap);
  SetDeviceDriver(pdfium::MakeUnique<CFX_SkiaDeviceDriver>(
      pBitmap, bRgbByteOrder, pOriDevice, bGroupKnockout));
  return true;
}

#ifdef _SKIA_SUPPORT_
bool CFX_DefaultRenderDevice::AttachRecorder(SkPictureRecorder* recorder) {
  if (!recorder)
    return false;
  SetDeviceDriver(pdfium::MakeUnique<CFX_SkiaDeviceDriver>(recorder));
  return true;
}
#endif  // _SKIA_SUPPORT_

bool CFX_DefaultRenderDevice::Create(
    int width,
    int height,
    FXDIB_Format format,
    const CFX_RetainPtr<CFX_DIBitmap>& pOriDevice) {
  auto pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pBitmap->Create(width, height, format)) {
    return false;
  }
  SetBitmap(pBitmap);
  SetDeviceDriver(pdfium::MakeUnique<CFX_SkiaDeviceDriver>(pBitmap, false,
                                                           pOriDevice, false));
  return true;
}

CFX_DefaultRenderDevice::~CFX_DefaultRenderDevice() {
  Flush();
}

#ifdef _SKIA_SUPPORT_
void CFX_DefaultRenderDevice::DebugVerifyBitmapIsPreMultiplied() const {
#ifdef SK_DEBUG
  CFX_SkiaDeviceDriver* skDriver =
      static_cast<CFX_SkiaDeviceDriver*>(GetDeviceDriver());
  if (skDriver)
    skDriver->DebugVerifyBitmapIsPreMultiplied();
#endif  // SK_DEBUG
}

bool CFX_DefaultRenderDevice::SetBitsWithMask(
    const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
    const CFX_RetainPtr<CFX_DIBSource>& pMask,
    int left,
    int top,
    int bitmap_alpha,
    int blend_type) {
  CFX_SkiaDeviceDriver* skDriver =
      static_cast<CFX_SkiaDeviceDriver*>(GetDeviceDriver());
  if (skDriver)
    return skDriver->SetBitsWithMask(pBitmap, pMask, left, top, bitmap_alpha,
                                     blend_type);
  return false;
}
#endif  // _SKIA_SUPPORT_

void CFX_DIBSource::DebugVerifyBitmapIsPreMultiplied(void* opt) const {
#ifdef SK_DEBUG
  SkASSERT(32 == GetBPP());
  const uint32_t* buffer = (const uint32_t*)(opt ? opt : GetBuffer());
  int width = GetWidth();
  int height = GetHeight();
  // verify that input is really premultiplied
  for (int y = 0; y < height; ++y) {
    const uint32_t* srcRow = buffer + y * width;
    for (int x = 0; x < width; ++x) {
      uint8_t a = SkGetPackedA32(srcRow[x]);
      uint8_t r = SkGetPackedR32(srcRow[x]);
      uint8_t g = SkGetPackedG32(srcRow[x]);
      uint8_t b = SkGetPackedB32(srcRow[x]);
      SkA32Assert(a);
      SkASSERT(r <= a);
      SkASSERT(g <= a);
      SkASSERT(b <= a);
    }
  }
#endif  // SK_DEBUG
}
