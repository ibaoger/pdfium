// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pdfwindow/cpwl_scroll_bar.h"

#include <algorithm>
#include <sstream>

#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/cfx_renderdevice.h"
#include "fpdfsdk/pdfwindow/cpwl_utils.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"

namespace {

constexpr float kButtonWidth = 9.0f;
constexpr float kPosButtonMinWidth = 2.0f;
constexpr float kTriangleHalfLength = 2.0f;

}  // namespace

#define PWL_DEFAULT_HEAVYGRAYCOLOR CPWL_Color(COLORTYPE_GRAY, 0.50)

PWL_FLOATRANGE::PWL_FLOATRANGE() {
  Default();
}

PWL_FLOATRANGE::PWL_FLOATRANGE(float min, float max) {
  Set(min, max);
}

void PWL_FLOATRANGE::Default() {
  fMin = 0;
  fMax = 0;
}

void PWL_FLOATRANGE::Set(float min, float max) {
  if (min > max) {
    fMin = max;
    fMax = min;
  } else {
    fMin = min;
    fMax = max;
  }
}

bool PWL_FLOATRANGE::In(float x) const {
  return (IsFloatBigger(x, fMin) || IsFloatEqual(x, fMin)) &&
         (IsFloatSmaller(x, fMax) || IsFloatEqual(x, fMax));
}

float PWL_FLOATRANGE::GetWidth() const {
  return fMax - fMin;
}

PWL_SCROLL_PRIVATEDATA::PWL_SCROLL_PRIVATEDATA() {
  Default();
}

void PWL_SCROLL_PRIVATEDATA::Default() {
  ScrollRange.Default();
  fScrollPos = ScrollRange.fMin;
  fClientWidth = 0;
  fBigStep = 10;
  fSmallStep = 1;
}

void PWL_SCROLL_PRIVATEDATA::SetScrollRange(float min, float max) {
  ScrollRange.Set(min, max);

  if (IsFloatSmaller(fScrollPos, ScrollRange.fMin))
    fScrollPos = ScrollRange.fMin;
  if (IsFloatBigger(fScrollPos, ScrollRange.fMax))
    fScrollPos = ScrollRange.fMax;
}

void PWL_SCROLL_PRIVATEDATA::SetClientWidth(float width) {
  fClientWidth = width;
}

void PWL_SCROLL_PRIVATEDATA::SetSmallStep(float step) {
  fSmallStep = step;
}

void PWL_SCROLL_PRIVATEDATA::SetBigStep(float step) {
  fBigStep = step;
}

bool PWL_SCROLL_PRIVATEDATA::SetPos(float pos) {
  if (ScrollRange.In(pos)) {
    fScrollPos = pos;
    return true;
  }
  return false;
}

void PWL_SCROLL_PRIVATEDATA::AddSmall() {
  if (!SetPos(fScrollPos + fSmallStep))
    SetPos(ScrollRange.fMax);
}

void PWL_SCROLL_PRIVATEDATA::SubSmall() {
  if (!SetPos(fScrollPos - fSmallStep))
    SetPos(ScrollRange.fMin);
}

void PWL_SCROLL_PRIVATEDATA::AddBig() {
  if (!SetPos(fScrollPos + fBigStep))
    SetPos(ScrollRange.fMax);
}

void PWL_SCROLL_PRIVATEDATA::SubBig() {
  if (!SetPos(fScrollPos - fBigStep))
    SetPos(ScrollRange.fMin);
}

CPWL_SBButton::CPWL_SBButton(PWL_SCROLLBAR_TYPE eScrollBarType,
                             PWL_SBBUTTON_TYPE eButtonType) {
  m_eScrollBarType = eScrollBarType;
  m_eSBButtonType = eButtonType;

  m_bMouseDown = false;
}

CPWL_SBButton::~CPWL_SBButton() {}

CFX_ByteString CPWL_SBButton::GetClassName() const {
  return "CPWL_SBButton";
}

void CPWL_SBButton::OnCreate(PWL_CREATEPARAM& cp) {
  cp.eCursorType = FXCT_ARROW;
}

void CPWL_SBButton::GetThisAppearanceStream(std::ostringstream* psAppStream) {
  CPWL_Wnd::GetThisAppearanceStream(psAppStream);

  if (!IsVisible())
    return;

  CFX_FloatRect rectWnd = GetWindowRect();
  if (rectWnd.IsEmpty())
    return;

  *psAppStream << "q\n";

  CFX_PointF ptCenter = GetCenterPoint();

  switch (m_eScrollBarType) {
    case SBT_HSCROLL:
      switch (m_eSBButtonType) {
        case PSBT_MIN: {
          CFX_PointF pt1(ptCenter.x - kTriangleHalfLength * 0.5f, ptCenter.y);
          CFX_PointF pt2(ptCenter.x + kTriangleHalfLength * 0.5f,
                         ptCenter.y + kTriangleHalfLength);
          CFX_PointF pt3(ptCenter.x + kTriangleHalfLength * 0.5f,
                         ptCenter.y - kTriangleHalfLength);

          if (rectWnd.right - rectWnd.left > kTriangleHalfLength * 2 &&
              rectWnd.top - rectWnd.bottom > kTriangleHalfLength) {
            *psAppStream << "0 g\n"
                         << pt1.x << " " << pt1.y << " m\n"
                         << pt2.x << " " << pt2.y << " l\n"
                         << pt3.x << " " << pt3.y << " l\n"
                         << pt1.x << " " << pt1.y << " l f\n";
          }
        } break;
        case PSBT_MAX: {
          CFX_PointF pt1(ptCenter.x + kTriangleHalfLength * 0.5f, ptCenter.y);
          CFX_PointF pt2(ptCenter.x - kTriangleHalfLength * 0.5f,
                         ptCenter.y + kTriangleHalfLength);
          CFX_PointF pt3(ptCenter.x - kTriangleHalfLength * 0.5f,
                         ptCenter.y - kTriangleHalfLength);

          if (rectWnd.right - rectWnd.left > kTriangleHalfLength * 2 &&
              rectWnd.top - rectWnd.bottom > kTriangleHalfLength) {
            *psAppStream << "0 g\n"
                         << pt1.x << " " << pt1.y << " m\n"
                         << pt2.x << " " << pt2.y << " l\n"
                         << pt3.x << " " << pt3.y << " l\n"
                         << pt1.x << " " << pt1.y << " l f\n";
          }
        } break;
        default:
          break;
      }
      break;
    case SBT_VSCROLL:
      switch (m_eSBButtonType) {
        case PSBT_MIN: {
          CFX_PointF pt1(ptCenter.x - kTriangleHalfLength,
                         ptCenter.y - kTriangleHalfLength * 0.5f);
          CFX_PointF pt2(ptCenter.x + kTriangleHalfLength,
                         ptCenter.y - kTriangleHalfLength * 0.5f);
          CFX_PointF pt3(ptCenter.x, ptCenter.y + kTriangleHalfLength * 0.5f);

          if (rectWnd.right - rectWnd.left > kTriangleHalfLength * 2 &&
              rectWnd.top - rectWnd.bottom > kTriangleHalfLength) {
            *psAppStream << "0 g\n"
                         << pt1.x << " " << pt1.y << " m\n"
                         << pt2.x << " " << pt2.y << " l\n"
                         << pt3.x << " " << pt3.y << " l\n"
                         << pt1.x << " " << pt1.y << " l f\n";
          }
        } break;
        case PSBT_MAX: {
          CFX_PointF pt1(ptCenter.x - kTriangleHalfLength,
                         ptCenter.y + kTriangleHalfLength * 0.5f);
          CFX_PointF pt2(ptCenter.x + kTriangleHalfLength,
                         ptCenter.y + kTriangleHalfLength * 0.5f);
          CFX_PointF pt3(ptCenter.x, ptCenter.y - kTriangleHalfLength * 0.5f);

          if (rectWnd.right - rectWnd.left > kTriangleHalfLength * 2 &&
              rectWnd.top - rectWnd.bottom > kTriangleHalfLength) {
            *psAppStream << "0 g\n"
                         << pt1.x << " " << pt1.y << " m\n"
                         << pt2.x << " " << pt2.y << " l\n"
                         << pt3.x << " " << pt3.y << " l\n"
                         << pt1.x << " " << pt1.y << " l f\n";
          }
        } break;
        default:
          break;
      }
      break;
    default:
      break;
  }

  *psAppStream << "Q\n";
}

void CPWL_SBButton::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                       CFX_Matrix* pUser2Device) {
  if (!IsVisible())
    return;

  CFX_FloatRect rectWnd = GetWindowRect();
  if (rectWnd.IsEmpty())
    return;

  CFX_PointF ptCenter = GetCenterPoint();
  int32_t nTransparency = GetTransparency();

  switch (m_eScrollBarType) {
    case SBT_HSCROLL:
      CPWL_Wnd::DrawThisAppearance(pDevice, pUser2Device);
      switch (m_eSBButtonType) {
        case PSBT_MIN: {
          CFX_PointF pt1(ptCenter.x - kTriangleHalfLength * 0.5f, ptCenter.y);
          CFX_PointF pt2(ptCenter.x + kTriangleHalfLength * 0.5f,
                         ptCenter.y + kTriangleHalfLength);
          CFX_PointF pt3(ptCenter.x + kTriangleHalfLength * 0.5f,
                         ptCenter.y - kTriangleHalfLength);

          if (rectWnd.right - rectWnd.left > kTriangleHalfLength * 2 &&
              rectWnd.top - rectWnd.bottom > kTriangleHalfLength) {
            CFX_PathData path;
            path.AppendPoint(pt1, FXPT_TYPE::MoveTo, false);
            path.AppendPoint(pt2, FXPT_TYPE::LineTo, false);
            path.AppendPoint(pt3, FXPT_TYPE::LineTo, false);
            path.AppendPoint(pt1, FXPT_TYPE::LineTo, false);

            pDevice->DrawPath(&path, pUser2Device, nullptr,
                              PWL_DEFAULT_BLACKCOLOR.ToFXColor(nTransparency),
                              0, FXFILL_ALTERNATE);
          }
        } break;
        case PSBT_MAX: {
          CFX_PointF pt1(ptCenter.x + kTriangleHalfLength * 0.5f, ptCenter.y);
          CFX_PointF pt2(ptCenter.x - kTriangleHalfLength * 0.5f,
                         ptCenter.y + kTriangleHalfLength);
          CFX_PointF pt3(ptCenter.x - kTriangleHalfLength * 0.5f,
                         ptCenter.y - kTriangleHalfLength);

          if (rectWnd.right - rectWnd.left > kTriangleHalfLength * 2 &&
              rectWnd.top - rectWnd.bottom > kTriangleHalfLength) {
            CFX_PathData path;
            path.AppendPoint(pt1, FXPT_TYPE::MoveTo, false);
            path.AppendPoint(pt2, FXPT_TYPE::LineTo, false);
            path.AppendPoint(pt3, FXPT_TYPE::LineTo, false);
            path.AppendPoint(pt1, FXPT_TYPE::LineTo, false);

            pDevice->DrawPath(&path, pUser2Device, nullptr,
                              PWL_DEFAULT_BLACKCOLOR.ToFXColor(nTransparency),
                              0, FXFILL_ALTERNATE);
          }
        } break;
        default:
          break;
      }
      break;
    case SBT_VSCROLL:
      switch (m_eSBButtonType) {
        case PSBT_MIN: {
          // draw border
          CFX_FloatRect rcDraw = rectWnd;
          CPWL_Utils::DrawStrokeRect(pDevice, pUser2Device, rcDraw,
                                     ArgbEncode(nTransparency, 100, 100, 100),
                                     0.0f);

          // draw inner border
          rcDraw = CPWL_Utils::DeflateRect(rectWnd, 0.5f);
          CPWL_Utils::DrawStrokeRect(pDevice, pUser2Device, rcDraw,
                                     ArgbEncode(nTransparency, 255, 255, 255),
                                     1.0f);

          // draw background

          rcDraw = CPWL_Utils::DeflateRect(rectWnd, 1.0f);

          if (IsEnabled())
            CPWL_Utils::DrawShadow(pDevice, pUser2Device, true, false, rcDraw,
                                   nTransparency, 80, 220);
          else
            CPWL_Utils::DrawFillRect(pDevice, pUser2Device, rcDraw,
                                     ArgbEncode(255, 255, 255, 255));

          // draw arrow

          if (rectWnd.top - rectWnd.bottom > 6.0f) {
            float fX = rectWnd.left + 1.5f;
            float fY = rectWnd.bottom;
            CFX_PointF pts[7] = {CFX_PointF(fX + 2.5f, fY + 4.0f),
                                 CFX_PointF(fX + 2.5f, fY + 3.0f),
                                 CFX_PointF(fX + 4.5f, fY + 5.0f),
                                 CFX_PointF(fX + 6.5f, fY + 3.0f),
                                 CFX_PointF(fX + 6.5f, fY + 4.0f),
                                 CFX_PointF(fX + 4.5f, fY + 6.0f),
                                 CFX_PointF(fX + 2.5f, fY + 4.0f)};

            if (IsEnabled())
              CPWL_Utils::DrawFillArea(
                  pDevice, pUser2Device, pts, 7,
                  ArgbEncode(nTransparency, 255, 255, 255));
            else
              CPWL_Utils::DrawFillArea(
                  pDevice, pUser2Device, pts, 7,
                  PWL_DEFAULT_HEAVYGRAYCOLOR.ToFXColor(255));
          }
        } break;
        case PSBT_MAX: {
          // draw border
          CFX_FloatRect rcDraw = rectWnd;
          CPWL_Utils::DrawStrokeRect(pDevice, pUser2Device, rcDraw,
                                     ArgbEncode(nTransparency, 100, 100, 100),
                                     0.0f);

          // draw inner border
          rcDraw = CPWL_Utils::DeflateRect(rectWnd, 0.5f);
          CPWL_Utils::DrawStrokeRect(pDevice, pUser2Device, rcDraw,
                                     ArgbEncode(nTransparency, 255, 255, 255),
                                     1.0f);

          // draw background
          rcDraw = CPWL_Utils::DeflateRect(rectWnd, 1.0f);
          if (IsEnabled())
            CPWL_Utils::DrawShadow(pDevice, pUser2Device, true, false, rcDraw,
                                   nTransparency, 80, 220);
          else
            CPWL_Utils::DrawFillRect(pDevice, pUser2Device, rcDraw,
                                     ArgbEncode(255, 255, 255, 255));

          // draw arrow

          if (rectWnd.top - rectWnd.bottom > 6.0f) {
            float fX = rectWnd.left + 1.5f;
            float fY = rectWnd.bottom;

            CFX_PointF pts[7] = {CFX_PointF(fX + 2.5f, fY + 5.0f),
                                 CFX_PointF(fX + 2.5f, fY + 6.0f),
                                 CFX_PointF(fX + 4.5f, fY + 4.0f),
                                 CFX_PointF(fX + 6.5f, fY + 6.0f),
                                 CFX_PointF(fX + 6.5f, fY + 5.0f),
                                 CFX_PointF(fX + 4.5f, fY + 3.0f),
                                 CFX_PointF(fX + 2.5f, fY + 5.0f)};

            if (IsEnabled())
              CPWL_Utils::DrawFillArea(
                  pDevice, pUser2Device, pts, 7,
                  ArgbEncode(nTransparency, 255, 255, 255));
            else
              CPWL_Utils::DrawFillArea(
                  pDevice, pUser2Device, pts, 7,
                  PWL_DEFAULT_HEAVYGRAYCOLOR.ToFXColor(255));
          }
        } break;
        case PSBT_POS: {
          // draw border
          CFX_FloatRect rcDraw = rectWnd;
          CPWL_Utils::DrawStrokeRect(pDevice, pUser2Device, rcDraw,
                                     ArgbEncode(nTransparency, 100, 100, 100),
                                     0.0f);

          // draw inner border
          rcDraw = CPWL_Utils::DeflateRect(rectWnd, 0.5f);
          CPWL_Utils::DrawStrokeRect(pDevice, pUser2Device, rcDraw,
                                     ArgbEncode(nTransparency, 255, 255, 255),
                                     1.0f);

          if (IsEnabled()) {
            // draw shadow effect

            CFX_PointF ptTop = CFX_PointF(rectWnd.left, rectWnd.top - 1.0f);
            CFX_PointF ptBottom =
                CFX_PointF(rectWnd.left, rectWnd.bottom + 1.0f);

            ptTop.x += 1.5f;
            ptBottom.x += 1.5f;

            CPWL_Utils::DrawStrokeLine(pDevice, pUser2Device, ptTop, ptBottom,
                                       ArgbEncode(nTransparency, 210, 210, 210),
                                       1.0f);

            ptTop.x += 1.0f;
            ptBottom.x += 1.0f;

            CPWL_Utils::DrawStrokeLine(pDevice, pUser2Device, ptTop, ptBottom,
                                       ArgbEncode(nTransparency, 220, 220, 220),
                                       1.0f);

            ptTop.x += 1.0f;
            ptBottom.x += 1.0f;

            CPWL_Utils::DrawStrokeLine(pDevice, pUser2Device, ptTop, ptBottom,
                                       ArgbEncode(nTransparency, 240, 240, 240),
                                       1.0f);

            ptTop.x += 1.0f;
            ptBottom.x += 1.0f;

            CPWL_Utils::DrawStrokeLine(pDevice, pUser2Device, ptTop, ptBottom,
                                       ArgbEncode(nTransparency, 240, 240, 240),
                                       1.0f);

            ptTop.x += 1.0f;
            ptBottom.x += 1.0f;

            CPWL_Utils::DrawStrokeLine(pDevice, pUser2Device, ptTop, ptBottom,
                                       ArgbEncode(nTransparency, 210, 210, 210),
                                       1.0f);

            ptTop.x += 1.0f;
            ptBottom.x += 1.0f;

            CPWL_Utils::DrawStrokeLine(pDevice, pUser2Device, ptTop, ptBottom,
                                       ArgbEncode(nTransparency, 180, 180, 180),
                                       1.0f);

            ptTop.x += 1.0f;
            ptBottom.x += 1.0f;

            CPWL_Utils::DrawStrokeLine(pDevice, pUser2Device, ptTop, ptBottom,
                                       ArgbEncode(nTransparency, 150, 150, 150),
                                       1.0f);

            ptTop.x += 1.0f;
            ptBottom.x += 1.0f;

            CPWL_Utils::DrawStrokeLine(pDevice, pUser2Device, ptTop, ptBottom,
                                       ArgbEncode(nTransparency, 150, 150, 150),
                                       1.0f);

            ptTop.x += 1.0f;
            ptBottom.x += 1.0f;

            CPWL_Utils::DrawStrokeLine(pDevice, pUser2Device, ptTop, ptBottom,
                                       ArgbEncode(nTransparency, 180, 180, 180),
                                       1.0f);

            ptTop.x += 1.0f;
            ptBottom.x += 1.0f;

            CPWL_Utils::DrawStrokeLine(pDevice, pUser2Device, ptTop, ptBottom,
                                       ArgbEncode(nTransparency, 210, 210, 210),
                                       1.0f);
          } else {
            CPWL_Utils::DrawFillRect(pDevice, pUser2Device, rcDraw,
                                     ArgbEncode(255, 255, 255, 255));
          }

          // draw friction

          if (rectWnd.Height() > 8.0f) {
            FX_COLORREF crStroke = ArgbEncode(nTransparency, 120, 120, 120);
            if (!IsEnabled())
              crStroke = PWL_DEFAULT_HEAVYGRAYCOLOR.ToFXColor(255);

            float nFrictionWidth = 5.0f;
            float nFrictionHeight = 5.5f;

            CFX_PointF ptLeft =
                CFX_PointF(ptCenter.x - nFrictionWidth / 2.0f,
                           ptCenter.y - nFrictionHeight / 2.0f + 0.5f);
            CFX_PointF ptRight =
                CFX_PointF(ptCenter.x + nFrictionWidth / 2.0f,
                           ptCenter.y - nFrictionHeight / 2.0f + 0.5f);

            CPWL_Utils::DrawStrokeLine(pDevice, pUser2Device, ptLeft, ptRight,
                                       crStroke, 1.0f);

            ptLeft.y += 2.0f;
            ptRight.y += 2.0f;

            CPWL_Utils::DrawStrokeLine(pDevice, pUser2Device, ptLeft, ptRight,
                                       crStroke, 1.0f);

            ptLeft.y += 2.0f;
            ptRight.y += 2.0f;

            CPWL_Utils::DrawStrokeLine(pDevice, pUser2Device, ptLeft, ptRight,
                                       crStroke, 1.0f);
          }
        } break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

bool CPWL_SBButton::OnLButtonDown(const CFX_PointF& point, uint32_t nFlag) {
  CPWL_Wnd::OnLButtonDown(point, nFlag);

  if (CPWL_Wnd* pParent = GetParentWindow())
    pParent->NotifyLButtonDown(this, point);

  m_bMouseDown = true;
  SetCapture();

  return true;
}

bool CPWL_SBButton::OnLButtonUp(const CFX_PointF& point, uint32_t nFlag) {
  CPWL_Wnd::OnLButtonUp(point, nFlag);

  if (CPWL_Wnd* pParent = GetParentWindow())
    pParent->NotifyLButtonUp(this, point);

  m_bMouseDown = false;
  ReleaseCapture();

  return true;
}

bool CPWL_SBButton::OnMouseMove(const CFX_PointF& point, uint32_t nFlag) {
  CPWL_Wnd::OnMouseMove(point, nFlag);

  if (CPWL_Wnd* pParent = GetParentWindow())
    pParent->NotifyMouseMove(this, point);

  return true;
}

CPWL_ScrollBar::CPWL_ScrollBar(PWL_SCROLLBAR_TYPE sbType)
    : m_sbType(sbType),
      m_pMinButton(nullptr),
      m_pMaxButton(nullptr),
      m_pPosButton(nullptr),
      m_bMouseDown(false),
      m_bMinOrMax(false),
      m_bNotifyForever(true) {}

CPWL_ScrollBar::~CPWL_ScrollBar() {}

CFX_ByteString CPWL_ScrollBar::GetClassName() const {
  return "CPWL_ScrollBar";
}

void CPWL_ScrollBar::OnCreate(PWL_CREATEPARAM& cp) {
  cp.eCursorType = FXCT_ARROW;
}

void CPWL_ScrollBar::OnDestroy() {
  // Until cleanup takes place in the virtual destructor for CPWL_Wnd
  // subclasses, implement the virtual OnDestroy method that does the
  // cleanup first, then invokes the superclass OnDestroy ... gee,
  // like a dtor would.
  m_pMinButton.Release();
  m_pMaxButton.Release();
  m_pPosButton.Release();
  CPWL_Wnd::OnDestroy();
}

void CPWL_ScrollBar::RePosChildWnd() {
  CFX_FloatRect rcClient = GetClientRect();
  CFX_FloatRect rcMinButton, rcMaxButton;
  float fBWidth = 0;

  switch (m_sbType) {
    case SBT_HSCROLL:
      if (rcClient.right - rcClient.left >
          kButtonWidth * 2 + kPosButtonMinWidth + 2) {
        rcMinButton = CFX_FloatRect(rcClient.left, rcClient.bottom,
                                    rcClient.left + kButtonWidth, rcClient.top);
        rcMaxButton =
            CFX_FloatRect(rcClient.right - kButtonWidth, rcClient.bottom,
                          rcClient.right, rcClient.top);
      } else {
        fBWidth = (rcClient.right - rcClient.left - kPosButtonMinWidth - 2) / 2;

        if (fBWidth > 0) {
          rcMinButton = CFX_FloatRect(rcClient.left, rcClient.bottom,
                                      rcClient.left + fBWidth, rcClient.top);
          rcMaxButton = CFX_FloatRect(rcClient.right - fBWidth, rcClient.bottom,
                                      rcClient.right, rcClient.top);
        } else {
          SetVisible(false);
        }
      }
      break;
    case SBT_VSCROLL:
      if (IsFloatBigger(rcClient.top - rcClient.bottom,
                        kButtonWidth * 2 + kPosButtonMinWidth + 2)) {
        rcMinButton = CFX_FloatRect(rcClient.left, rcClient.top - kButtonWidth,
                                    rcClient.right, rcClient.top);
        rcMaxButton =
            CFX_FloatRect(rcClient.left, rcClient.bottom, rcClient.right,
                          rcClient.bottom + kButtonWidth);
      } else {
        fBWidth = (rcClient.top - rcClient.bottom - kPosButtonMinWidth - 2) / 2;

        if (IsFloatBigger(fBWidth, 0)) {
          rcMinButton = CFX_FloatRect(rcClient.left, rcClient.top - fBWidth,
                                      rcClient.right, rcClient.top);
          rcMaxButton =
              CFX_FloatRect(rcClient.left, rcClient.bottom, rcClient.right,
                            rcClient.bottom + fBWidth);
        } else {
          SetVisible(false);
        }
      }
      break;
  }

  if (m_pMinButton)
    m_pMinButton->Move(rcMinButton, true, false);
  if (m_pMaxButton)
    m_pMaxButton->Move(rcMaxButton, true, false);
  MovePosButton(false);
}

void CPWL_ScrollBar::GetThisAppearanceStream(std::ostringstream* psAppStream) {
  CFX_FloatRect rectWnd = GetWindowRect();

  if (!IsVisible() || rectWnd.IsEmpty())
    return;

  *psAppStream << "q\n"
               << "0 w\n"
               << CPWL_Utils::GetColorAppStream(GetBackgroundColor(), true)
               << rectWnd.left << " " << rectWnd.bottom << " "
               << rectWnd.right - rectWnd.left << " "
               << rectWnd.top - rectWnd.bottom << " re b Q\n";
}

void CPWL_ScrollBar::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                        CFX_Matrix* pUser2Device) {
  CFX_FloatRect rectWnd = GetWindowRect();

  if (IsVisible() && !rectWnd.IsEmpty()) {
    CPWL_Utils::DrawFillRect(pDevice, pUser2Device, rectWnd,
                             GetBackgroundColor(), GetTransparency());

    CPWL_Utils::DrawStrokeLine(
        pDevice, pUser2Device,
        CFX_PointF(rectWnd.left + 2.0f, rectWnd.top - 2.0f),
        CFX_PointF(rectWnd.left + 2.0f, rectWnd.bottom + 2.0f),
        ArgbEncode(GetTransparency(), 100, 100, 100), 1.0f);

    CPWL_Utils::DrawStrokeLine(
        pDevice, pUser2Device,
        CFX_PointF(rectWnd.right - 2.0f, rectWnd.top - 2.0f),
        CFX_PointF(rectWnd.right - 2.0f, rectWnd.bottom + 2.0f),
        ArgbEncode(GetTransparency(), 100, 100, 100), 1.0f);
  }
}

bool CPWL_ScrollBar::OnLButtonDown(const CFX_PointF& point, uint32_t nFlag) {
  CPWL_Wnd::OnLButtonDown(point, nFlag);

  if (HasFlag(PWS_AUTOTRANSPARENT)) {
    if (GetTransparency() != 255) {
      SetTransparency(255);
      InvalidateRect();
    }
  }

  CFX_FloatRect rcMinArea, rcMaxArea;

  if (m_pPosButton && m_pPosButton->IsVisible()) {
    CFX_FloatRect rcClient = GetClientRect();
    CFX_FloatRect rcPosButton = m_pPosButton->GetWindowRect();

    switch (m_sbType) {
      case SBT_HSCROLL:
        rcMinArea = CFX_FloatRect(rcClient.left + kButtonWidth, rcClient.bottom,
                                  rcPosButton.left, rcClient.top);
        rcMaxArea = CFX_FloatRect(rcPosButton.right, rcClient.bottom,
                                  rcClient.right - kButtonWidth, rcClient.top);

        break;
      case SBT_VSCROLL:
        rcMinArea = CFX_FloatRect(rcClient.left, rcPosButton.top,
                                  rcClient.right, rcClient.top - kButtonWidth);
        rcMaxArea = CFX_FloatRect(rcClient.left, rcClient.bottom + kButtonWidth,
                                  rcClient.right, rcPosButton.bottom);
        break;
    }

    rcMinArea.Normalize();
    rcMaxArea.Normalize();

    if (rcMinArea.Contains(point)) {
      m_sData.SubBig();
      MovePosButton(true);
      NotifyScrollWindow();
    }

    if (rcMaxArea.Contains(point)) {
      m_sData.AddBig();
      MovePosButton(true);
      NotifyScrollWindow();
    }
  }

  return true;
}

bool CPWL_ScrollBar::OnLButtonUp(const CFX_PointF& point, uint32_t nFlag) {
  CPWL_Wnd::OnLButtonUp(point, nFlag);

  if (HasFlag(PWS_AUTOTRANSPARENT)) {
    if (GetTransparency() != PWL_SCROLLBAR_TRANSPARENCY) {
      SetTransparency(PWL_SCROLLBAR_TRANSPARENCY);
      InvalidateRect();
    }
  }

  EndTimer();
  m_bMouseDown = false;

  return true;
}

void CPWL_ScrollBar::SetScrollInfo(const PWL_SCROLL_INFO& info) {
  if (info == m_OriginInfo)
    return;

  m_OriginInfo = info;
  float fMax =
      std::max(0.0f, info.fContentMax - info.fContentMin - info.fPlateWidth);
  SetScrollRange(0, fMax, info.fPlateWidth);
  SetScrollStep(info.fBigStep, info.fSmallStep);
}

void CPWL_ScrollBar::SetScrollPosition(float pos) {
  switch (m_sbType) {
    case SBT_HSCROLL:
      pos = pos - m_OriginInfo.fContentMin;
      break;
    case SBT_VSCROLL:
      pos = m_OriginInfo.fContentMax - pos;
      break;
  }
  SetScrollPos(pos);
}

void CPWL_ScrollBar::NotifyLButtonDown(CPWL_Wnd* child, const CFX_PointF& pos) {
  if (child == m_pMinButton)
    OnMinButtonLBDown(pos);
  else if (child == m_pMaxButton)
    OnMaxButtonLBDown(pos);
  else if (child == m_pPosButton)
    OnPosButtonLBDown(pos);
}

void CPWL_ScrollBar::NotifyLButtonUp(CPWL_Wnd* child, const CFX_PointF& pos) {
  if (child == m_pMinButton)
    OnMinButtonLBUp(pos);
  else if (child == m_pMaxButton)
    OnMaxButtonLBUp(pos);
  else if (child == m_pPosButton)
    OnPosButtonLBUp(pos);
}

void CPWL_ScrollBar::NotifyMouseMove(CPWL_Wnd* child, const CFX_PointF& pos) {
  if (child == m_pMinButton)
    OnMinButtonMouseMove(pos);
  else if (child == m_pMaxButton)
    OnMaxButtonMouseMove(pos);
  else if (child == m_pPosButton)
    OnPosButtonMouseMove(pos);
}

void CPWL_ScrollBar::CreateButtons(const PWL_CREATEPARAM& cp) {
  PWL_CREATEPARAM scp = cp;
  scp.pParentWnd = this;
  scp.dwBorderWidth = 2;
  scp.nBorderStyle = BorderStyle::BEVELED;

  scp.dwFlags =
      PWS_VISIBLE | PWS_CHILD | PWS_BORDER | PWS_BACKGROUND | PWS_NOREFRESHCLIP;

  if (!m_pMinButton) {
    m_pMinButton = new CPWL_SBButton(m_sbType, PSBT_MIN);
    m_pMinButton->Create(scp);
  }

  if (!m_pMaxButton) {
    m_pMaxButton = new CPWL_SBButton(m_sbType, PSBT_MAX);
    m_pMaxButton->Create(scp);
  }

  if (!m_pPosButton) {
    m_pPosButton = new CPWL_SBButton(m_sbType, PSBT_POS);
    m_pPosButton->SetVisible(false);
    m_pPosButton->Create(scp);
  }
}

float CPWL_ScrollBar::GetScrollBarWidth() const {
  if (!IsVisible())
    return 0;

  return PWL_SCROLLBAR_WIDTH;
}

void CPWL_ScrollBar::SetScrollRange(float fMin,
                                    float fMax,
                                    float fClientWidth) {
  if (m_pPosButton) {
    m_sData.SetScrollRange(fMin, fMax);
    m_sData.SetClientWidth(fClientWidth);

    if (IsFloatSmaller(m_sData.ScrollRange.GetWidth(), 0.0f)) {
      m_pPosButton->SetVisible(false);
    } else {
      m_pPosButton->SetVisible(true);
      MovePosButton(true);
    }
  }
}

void CPWL_ScrollBar::SetScrollPos(float fPos) {
  float fOldPos = m_sData.fScrollPos;
  m_sData.SetPos(fPos);
  if (!IsFloatEqual(m_sData.fScrollPos, fOldPos))
    MovePosButton(true);
}

void CPWL_ScrollBar::SetScrollStep(float fBigStep, float fSmallStep) {
  m_sData.SetBigStep(fBigStep);
  m_sData.SetSmallStep(fSmallStep);
}

void CPWL_ScrollBar::MovePosButton(bool bRefresh) {
  ASSERT(m_pMinButton);
  ASSERT(m_pMaxButton);

  if (m_pPosButton->IsVisible()) {
    CFX_FloatRect rcClient;
    CFX_FloatRect rcPosArea, rcPosButton;

    rcClient = GetClientRect();
    rcPosArea = GetScrollArea();

    float fLeft, fRight, fTop, fBottom;

    switch (m_sbType) {
      case SBT_HSCROLL:
        fLeft = TrueToFace(m_sData.fScrollPos);
        fRight = TrueToFace(m_sData.fScrollPos + m_sData.fClientWidth);

        if (fRight - fLeft < kPosButtonMinWidth)
          fRight = fLeft + kPosButtonMinWidth;

        if (fRight > rcPosArea.right) {
          fRight = rcPosArea.right;
          fLeft = fRight - kPosButtonMinWidth;
        }

        rcPosButton =
            CFX_FloatRect(fLeft, rcPosArea.bottom, fRight, rcPosArea.top);

        break;
      case SBT_VSCROLL:
        fBottom = TrueToFace(m_sData.fScrollPos + m_sData.fClientWidth);
        fTop = TrueToFace(m_sData.fScrollPos);

        if (IsFloatSmaller(fTop - fBottom, kPosButtonMinWidth))
          fBottom = fTop - kPosButtonMinWidth;

        if (IsFloatSmaller(fBottom, rcPosArea.bottom)) {
          fBottom = rcPosArea.bottom;
          fTop = fBottom + kPosButtonMinWidth;
        }

        rcPosButton =
            CFX_FloatRect(rcPosArea.left, fBottom, rcPosArea.right, fTop);

        break;
    }

    m_pPosButton->Move(rcPosButton, true, bRefresh);
  }
}

void CPWL_ScrollBar::OnMinButtonLBDown(const CFX_PointF& point) {
  m_sData.SubSmall();
  MovePosButton(true);
  NotifyScrollWindow();

  m_bMinOrMax = true;

  EndTimer();
  BeginTimer(100);
}

void CPWL_ScrollBar::OnMinButtonLBUp(const CFX_PointF& point) {}

void CPWL_ScrollBar::OnMinButtonMouseMove(const CFX_PointF& point) {}

void CPWL_ScrollBar::OnMaxButtonLBDown(const CFX_PointF& point) {
  m_sData.AddSmall();
  MovePosButton(true);
  NotifyScrollWindow();

  m_bMinOrMax = false;

  EndTimer();
  BeginTimer(100);
}

void CPWL_ScrollBar::OnMaxButtonLBUp(const CFX_PointF& point) {}

void CPWL_ScrollBar::OnMaxButtonMouseMove(const CFX_PointF& point) {}

void CPWL_ScrollBar::OnPosButtonLBDown(const CFX_PointF& point) {
  m_bMouseDown = true;

  if (m_pPosButton) {
    CFX_FloatRect rcPosButton = m_pPosButton->GetWindowRect();

    switch (m_sbType) {
      case SBT_HSCROLL:
        m_nOldPos = point.x;
        m_fOldPosButton = rcPosButton.left;
        break;
      case SBT_VSCROLL:
        m_nOldPos = point.y;
        m_fOldPosButton = rcPosButton.top;
        break;
    }
  }
}

void CPWL_ScrollBar::OnPosButtonLBUp(const CFX_PointF& point) {
  if (m_bMouseDown) {
    if (!m_bNotifyForever)
      NotifyScrollWindow();
  }
  m_bMouseDown = false;
}

void CPWL_ScrollBar::OnPosButtonMouseMove(const CFX_PointF& point) {
  float fOldScrollPos = m_sData.fScrollPos;

  float fNewPos = 0;

  switch (m_sbType) {
    case SBT_HSCROLL:
      if (fabs(point.x - m_nOldPos) < 1)
        return;
      fNewPos = FaceToTrue(m_fOldPosButton + point.x - m_nOldPos);
      break;
    case SBT_VSCROLL:
      if (fabs(point.y - m_nOldPos) < 1)
        return;
      fNewPos = FaceToTrue(m_fOldPosButton + point.y - m_nOldPos);
      break;
  }

  if (m_bMouseDown) {
    switch (m_sbType) {
      case SBT_HSCROLL:

        if (IsFloatSmaller(fNewPos, m_sData.ScrollRange.fMin)) {
          fNewPos = m_sData.ScrollRange.fMin;
        }

        if (IsFloatBigger(fNewPos, m_sData.ScrollRange.fMax)) {
          fNewPos = m_sData.ScrollRange.fMax;
        }

        m_sData.SetPos(fNewPos);

        break;
      case SBT_VSCROLL:

        if (IsFloatSmaller(fNewPos, m_sData.ScrollRange.fMin)) {
          fNewPos = m_sData.ScrollRange.fMin;
        }

        if (IsFloatBigger(fNewPos, m_sData.ScrollRange.fMax)) {
          fNewPos = m_sData.ScrollRange.fMax;
        }

        m_sData.SetPos(fNewPos);

        break;
    }

    if (!IsFloatEqual(fOldScrollPos, m_sData.fScrollPos)) {
      MovePosButton(true);

      if (m_bNotifyForever)
        NotifyScrollWindow();
    }
  }
}

void CPWL_ScrollBar::NotifyScrollWindow() {
  CPWL_Wnd* pParent = GetParentWindow();
  if (!pParent || m_sbType != SBT_VSCROLL)
    return;

  pParent->ScrollWindowVertically(m_OriginInfo.fContentMax -
                                  m_sData.fScrollPos);
}

CFX_FloatRect CPWL_ScrollBar::GetScrollArea() const {
  CFX_FloatRect rcClient = GetClientRect();
  CFX_FloatRect rcArea;

  if (!m_pMinButton || !m_pMaxButton)
    return rcClient;

  CFX_FloatRect rcMin = m_pMinButton->GetWindowRect();
  CFX_FloatRect rcMax = m_pMaxButton->GetWindowRect();

  float fMinWidth = rcMin.right - rcMin.left;
  float fMinHeight = rcMin.top - rcMin.bottom;
  float fMaxWidth = rcMax.right - rcMax.left;
  float fMaxHeight = rcMax.top - rcMax.bottom;

  switch (m_sbType) {
    case SBT_HSCROLL:
      if (rcClient.right - rcClient.left > fMinWidth + fMaxWidth + 2) {
        rcArea = CFX_FloatRect(rcClient.left + fMinWidth + 1, rcClient.bottom,
                               rcClient.right - fMaxWidth - 1, rcClient.top);
      } else {
        rcArea = CFX_FloatRect(rcClient.left + fMinWidth + 1, rcClient.bottom,
                               rcClient.left + fMinWidth + 1, rcClient.top);
      }
      break;
    case SBT_VSCROLL:
      if (rcClient.top - rcClient.bottom > fMinHeight + fMaxHeight + 2) {
        rcArea = CFX_FloatRect(rcClient.left, rcClient.bottom + fMinHeight + 1,
                               rcClient.right, rcClient.top - fMaxHeight - 1);
      } else {
        rcArea =
            CFX_FloatRect(rcClient.left, rcClient.bottom + fMinHeight + 1,
                          rcClient.right, rcClient.bottom + fMinHeight + 1);
      }
      break;
  }

  rcArea.Normalize();

  return rcArea;
}

float CPWL_ScrollBar::TrueToFace(float fTrue) {
  CFX_FloatRect rcPosArea;
  rcPosArea = GetScrollArea();

  float fFactWidth = m_sData.ScrollRange.GetWidth() + m_sData.fClientWidth;
  fFactWidth = fFactWidth == 0 ? 1 : fFactWidth;

  float fFace = 0;

  switch (m_sbType) {
    case SBT_HSCROLL:
      fFace = rcPosArea.left +
              fTrue * (rcPosArea.right - rcPosArea.left) / fFactWidth;
      break;
    case SBT_VSCROLL:
      fFace = rcPosArea.top -
              fTrue * (rcPosArea.top - rcPosArea.bottom) / fFactWidth;
      break;
  }

  return fFace;
}

float CPWL_ScrollBar::FaceToTrue(float fFace) {
  CFX_FloatRect rcPosArea;
  rcPosArea = GetScrollArea();

  float fFactWidth = m_sData.ScrollRange.GetWidth() + m_sData.fClientWidth;
  fFactWidth = fFactWidth == 0 ? 1 : fFactWidth;

  float fTrue = 0;

  switch (m_sbType) {
    case SBT_HSCROLL:
      fTrue = (fFace - rcPosArea.left) * fFactWidth /
              (rcPosArea.right - rcPosArea.left);
      break;
    case SBT_VSCROLL:
      fTrue = (rcPosArea.top - fFace) * fFactWidth /
              (rcPosArea.top - rcPosArea.bottom);
      break;
  }

  return fTrue;
}

void CPWL_ScrollBar::CreateChildWnd(const PWL_CREATEPARAM& cp) {
  CreateButtons(cp);
}

void CPWL_ScrollBar::TimerProc() {
  PWL_SCROLL_PRIVATEDATA sTemp = m_sData;
  if (m_bMinOrMax)
    m_sData.SubSmall();
  else
    m_sData.AddSmall();

  if (sTemp != m_sData) {
    MovePosButton(true);
    NotifyScrollWindow();
  }
}
