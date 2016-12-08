// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_listbox.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/tto/fde_textout.h"
#include "xfa/fwl/core/cfwl_app.h"
#include "xfa/fwl/core/cfwl_msgkey.h"
#include "xfa/fwl/core/cfwl_msgmouse.h"
#include "xfa/fwl/core/cfwl_msgmousewheel.h"
#include "xfa/fwl/core/cfwl_themebackground.h"
#include "xfa/fwl/core/cfwl_themepart.h"
#include "xfa/fwl/core/cfwl_themetext.h"
#include "xfa/fwl/core/ifwl_themeprovider.h"

namespace {

const int kItemTextMargin = 2;

}  // namespace

CFWL_ListBox::CFWL_ListBox(const CFWL_App* app,
                           std::unique_ptr<CFWL_WidgetProperties> properties,
                           CFWL_Widget* pOuter)
    : CFWL_Widget(app, std::move(properties), pOuter),
      m_dwTTOStyles(0),
      m_iTTOAligns(0),
      m_hAnchor(nullptr),
      m_fScorllBarWidth(0),
      m_bLButtonDown(false),
      m_pScrollBarTP(nullptr) {
  m_rtClient.Reset();
  m_rtConent.Reset();
  m_rtStatic.Reset();
}

CFWL_ListBox::~CFWL_ListBox() {}

FWL_Type CFWL_ListBox::GetClassID() const {
  return FWL_Type::ListBox;
}

void CFWL_ListBox::GetWidgetRect(CFX_RectF& rect, bool bAutoSize) {
  if (!bAutoSize) {
    rect = m_pProperties->m_rtWidget;
    return;
  }

  rect.Set(0, 0, 0, 0);
  if (!m_pProperties->m_pThemeProvider)
    m_pProperties->m_pThemeProvider = GetAvailableTheme();

  CFX_SizeF fs = CalcSize(true);
  rect.Set(0, 0, fs.x, fs.y);
  CFWL_Widget::GetWidgetRect(rect, true);
}

void CFWL_ListBox::Update() {
  if (IsLocked())
    return;
  if (!m_pProperties->m_pThemeProvider)
    m_pProperties->m_pThemeProvider = GetAvailableTheme();

  switch (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_AlignMask) {
    case FWL_STYLEEXT_LTB_LeftAlign: {
      m_iTTOAligns = FDE_TTOALIGNMENT_CenterLeft;
      break;
    }
    case FWL_STYLEEXT_LTB_RightAlign: {
      m_iTTOAligns = FDE_TTOALIGNMENT_CenterRight;
      break;
    }
    case FWL_STYLEEXT_LTB_CenterAlign:
    default: {
      m_iTTOAligns = FDE_TTOALIGNMENT_Center;
      break;
    }
  }
  if (m_pProperties->m_dwStyleExes & FWL_WGTSTYLE_RTLReading)
    m_dwTTOStyles |= FDE_TTOSTYLE_RTL;

  m_dwTTOStyles |= FDE_TTOSTYLE_SingleLine;
  m_fScorllBarWidth = GetScrollWidth();
  CalcSize(false);
}

FWL_WidgetHit CFWL_ListBox::HitTest(FX_FLOAT fx, FX_FLOAT fy) {
  if (IsShowScrollBar(false)) {
    CFX_RectF rect;
    m_pHorzScrollBar->GetWidgetRect(rect, false);
    if (rect.Contains(fx, fy))
      return FWL_WidgetHit::HScrollBar;
  }
  if (IsShowScrollBar(true)) {
    CFX_RectF rect;
    m_pVertScrollBar->GetWidgetRect(rect, false);
    if (rect.Contains(fx, fy))
      return FWL_WidgetHit::VScrollBar;
  }
  if (m_rtClient.Contains(fx, fy))
    return FWL_WidgetHit::Client;
  return FWL_WidgetHit::Unknown;
}

void CFWL_ListBox::DrawWidget(CFX_Graphics* pGraphics,
                              const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return;
  if (!m_pProperties->m_pThemeProvider)
    return;

  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  pGraphics->SaveGraphState();
  if (HasBorder())
    DrawBorder(pGraphics, CFWL_Part::Border, pTheme, pMatrix);
  if (HasEdge())
    DrawEdge(pGraphics, CFWL_Part::Edge, pTheme, pMatrix);

  CFX_RectF rtClip(m_rtConent);
  if (IsShowScrollBar(false))
    rtClip.height -= m_fScorllBarWidth;
  if (IsShowScrollBar(true))
    rtClip.width -= m_fScorllBarWidth;
  if (pMatrix)
    pMatrix->TransformRect(rtClip);

  pGraphics->SetClipRect(rtClip);
  if ((m_pProperties->m_dwStyles & FWL_WGTSTYLE_NoBackground) == 0)
    DrawBkground(pGraphics, pTheme, pMatrix);

  DrawItems(pGraphics, pTheme, pMatrix);
  pGraphics->RestoreGraphState();
}

void CFWL_ListBox::SetThemeProvider(IFWL_ThemeProvider* pThemeProvider) {
  if (pThemeProvider)
    m_pProperties->m_pThemeProvider = pThemeProvider;
}

int32_t CFWL_ListBox::CountSelItems() {
  int32_t iRet = 0;
  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_ListItem* pItem = GetItem(this, i);
    if (!pItem)
      continue;

    uint32_t dwStyle = GetItemStyles(this, pItem);
    if (dwStyle & FWL_ITEMSTATE_LTB_Selected)
      iRet++;
  }
  return iRet;
}

CFWL_ListItem* CFWL_ListBox::GetSelItem(int32_t nIndexSel) {
  int32_t idx = GetSelIndex(nIndexSel);
  if (idx < 0)
    return nullptr;
  return GetItem(this, idx);
}

int32_t CFWL_ListBox::GetSelIndex(int32_t nIndex) {
  int32_t index = 0;
  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_ListItem* pItem = GetItem(this, i);
    if (!pItem)
      return -1;

    uint32_t dwStyle = GetItemStyles(this, pItem);
    if (dwStyle & FWL_ITEMSTATE_LTB_Selected) {
      if (index == nIndex)
        return i;
      index++;
    }
  }
  return -1;
}

void CFWL_ListBox::SetSelItem(CFWL_ListItem* pItem, bool bSelect) {
  if (!pItem) {
    if (bSelect) {
      SelectAll();
    } else {
      ClearSelection();
      SetFocusItem(nullptr);
    }
    return;
  }
  if (IsMultiSelection())
    SetSelectionDirect(pItem, bSelect);
  else
    SetSelection(pItem, pItem, bSelect);
}

CFX_WideString CFWL_ListBox::GetDataProviderItemText(CFWL_ListItem* pItem) {
  if (!pItem)
    return L"";
  return GetItemText(this, pItem);
}

CFWL_ListItem* CFWL_ListBox::GetListItem(CFWL_ListItem* pItem,
                                         uint32_t dwKeyCode) {
  CFWL_ListItem* hRet = nullptr;
  switch (dwKeyCode) {
    case FWL_VKEY_Up:
    case FWL_VKEY_Down:
    case FWL_VKEY_Home:
    case FWL_VKEY_End: {
      const bool bUp = dwKeyCode == FWL_VKEY_Up;
      const bool bDown = dwKeyCode == FWL_VKEY_Down;
      const bool bHome = dwKeyCode == FWL_VKEY_Home;
      int32_t iDstItem = -1;
      if (bUp || bDown) {
        int32_t index = GetItemIndex(this, pItem);
        iDstItem = dwKeyCode == FWL_VKEY_Up ? index - 1 : index + 1;
      } else if (bHome) {
        iDstItem = 0;
      } else {
        int32_t iCount = CountItems(this);
        iDstItem = iCount - 1;
      }
      hRet = GetItem(this, iDstItem);
      break;
    }
    default:
      break;
  }
  return hRet;
}

void CFWL_ListBox::SetSelection(CFWL_ListItem* hStart,
                                CFWL_ListItem* hEnd,
                                bool bSelected) {
  int32_t iStart = GetItemIndex(this, hStart);
  int32_t iEnd = GetItemIndex(this, hEnd);
  if (iStart > iEnd) {
    int32_t iTemp = iStart;
    iStart = iEnd;
    iEnd = iTemp;
  }
  if (bSelected) {
    int32_t iCount = CountItems(this);
    for (int32_t i = 0; i < iCount; i++) {
      CFWL_ListItem* pItem = GetItem(this, i);
      SetSelectionDirect(pItem, false);
    }
  }
  for (; iStart <= iEnd; iStart++) {
    CFWL_ListItem* pItem = GetItem(this, iStart);
    SetSelectionDirect(pItem, bSelected);
  }
}

void CFWL_ListBox::SetSelectionDirect(CFWL_ListItem* pItem, bool bSelect) {
  uint32_t dwOldStyle = GetItemStyles(this, pItem);
  bSelect ? dwOldStyle |= FWL_ITEMSTATE_LTB_Selected
          : dwOldStyle &= ~FWL_ITEMSTATE_LTB_Selected;
  SetItemStyles(this, pItem, dwOldStyle);
}

bool CFWL_ListBox::IsMultiSelection() const {
  return m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_MultiSelection;
}

bool CFWL_ListBox::IsItemSelected(CFWL_ListItem* pItem) {
  uint32_t dwState = GetItemStyles(this, pItem);
  return (dwState & FWL_ITEMSTATE_LTB_Selected) != 0;
}

void CFWL_ListBox::ClearSelection() {
  bool bMulti = IsMultiSelection();
  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_ListItem* pItem = GetItem(this, i);
    uint32_t dwState = GetItemStyles(this, pItem);
    if (!(dwState & FWL_ITEMSTATE_LTB_Selected))
      continue;
    SetSelectionDirect(pItem, false);
    if (!bMulti)
      return;
  }
}

void CFWL_ListBox::SelectAll() {
  if (!IsMultiSelection())
    return;

  int32_t iCount = CountItems(this);
  if (iCount <= 0)
    return;

  CFWL_ListItem* pItemStart = GetItem(this, 0);
  CFWL_ListItem* pItemEnd = GetItem(this, iCount - 1);
  SetSelection(pItemStart, pItemEnd, false);
}

CFWL_ListItem* CFWL_ListBox::GetFocusedItem() {
  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_ListItem* pItem = GetItem(this, i);
    if (!pItem)
      return nullptr;
    if (GetItemStyles(this, pItem) & FWL_ITEMSTATE_LTB_Focused)
      return pItem;
  }
  return nullptr;
}

void CFWL_ListBox::SetFocusItem(CFWL_ListItem* pItem) {
  CFWL_ListItem* hFocus = GetFocusedItem();
  if (pItem == hFocus)
    return;

  if (hFocus) {
    uint32_t dwStyle = GetItemStyles(this, hFocus);
    dwStyle &= ~FWL_ITEMSTATE_LTB_Focused;
    SetItemStyles(this, hFocus, dwStyle);
  }
  if (pItem) {
    uint32_t dwStyle = GetItemStyles(this, pItem);
    dwStyle |= FWL_ITEMSTATE_LTB_Focused;
    SetItemStyles(this, pItem, dwStyle);
  }
}

CFWL_ListItem* CFWL_ListBox::GetItemAtPoint(FX_FLOAT fx, FX_FLOAT fy) {
  fx -= m_rtConent.left, fy -= m_rtConent.top;
  FX_FLOAT fPosX = 0.0f;
  if (m_pHorzScrollBar)
    fPosX = m_pHorzScrollBar->GetPos();

  FX_FLOAT fPosY = 0.0;
  if (m_pVertScrollBar)
    fPosY = m_pVertScrollBar->GetPos();

  int32_t nCount = CountItems(this);
  for (int32_t i = 0; i < nCount; i++) {
    CFWL_ListItem* pItem = GetItem(this, i);
    if (!pItem)
      continue;

    CFX_RectF rtItem;
    GetItemRect(this, pItem, rtItem);
    rtItem.Offset(-fPosX, -fPosY);
    if (rtItem.Contains(fx, fy))
      return pItem;
  }
  return nullptr;
}

bool CFWL_ListBox::GetItemCheckRectInternal(CFWL_ListItem* pItem,
                                            CFX_RectF& rtCheck) {
  if (!(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_Check))
    return false;
  GetItemCheckRect(this, pItem, rtCheck);
  return true;
}

bool CFWL_ListBox::GetItemChecked(CFWL_ListItem* pItem) {
  if (!(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_Check))
    return false;
  return !!(GetItemCheckState(this, pItem) & FWL_ITEMSTATE_LTB_Checked);
}

bool CFWL_ListBox::SetItemChecked(CFWL_ListItem* pItem, bool bChecked) {
  if (!(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_Check))
    return false;

  SetItemCheckState(this, pItem, bChecked ? FWL_ITEMSTATE_LTB_Checked : 0);
  return true;
}

bool CFWL_ListBox::ScrollToVisible(CFWL_ListItem* pItem) {
  if (!m_pVertScrollBar)
    return false;

  CFX_RectF rtItem;
  GetItemRect(this, pItem, rtItem);

  bool bScroll = false;
  FX_FLOAT fPosY = m_pVertScrollBar->GetPos();
  rtItem.Offset(0, -fPosY + m_rtConent.top);
  if (rtItem.top < m_rtConent.top) {
    fPosY += rtItem.top - m_rtConent.top;
    bScroll = true;
  } else if (rtItem.bottom() > m_rtConent.bottom()) {
    fPosY += rtItem.bottom() - m_rtConent.bottom();
    bScroll = true;
  }
  if (!bScroll)
    return false;

  m_pVertScrollBar->SetPos(fPosY);
  m_pVertScrollBar->SetTrackPos(fPosY);
  Repaint(&m_rtClient);
  return true;
}

void CFWL_ListBox::DrawBkground(CFX_Graphics* pGraphics,
                                IFWL_ThemeProvider* pTheme,
                                const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return;
  if (!pTheme)
    return;

  CFWL_ThemeBackground param;
  param.m_pWidget = this;
  param.m_iPart = CFWL_Part::Background;
  param.m_dwStates = 0;
  param.m_pGraphics = pGraphics;
  param.m_matrix.Concat(*pMatrix);
  param.m_rtPart = m_rtClient;
  if (IsShowScrollBar(false) && IsShowScrollBar(true))
    param.m_pData = &m_rtStatic;
  if (!IsEnabled())
    param.m_dwStates = CFWL_PartState_Disabled;

  pTheme->DrawBackground(&param);
}

void CFWL_ListBox::DrawItems(CFX_Graphics* pGraphics,
                             IFWL_ThemeProvider* pTheme,
                             const CFX_Matrix* pMatrix) {
  FX_FLOAT fPosX = 0.0f;
  if (m_pHorzScrollBar)
    fPosX = m_pHorzScrollBar->GetPos();

  FX_FLOAT fPosY = 0.0f;
  if (m_pVertScrollBar)
    fPosY = m_pVertScrollBar->GetPos();

  CFX_RectF rtView(m_rtConent);
  if (m_pHorzScrollBar)
    rtView.height -= m_fScorllBarWidth;
  if (m_pVertScrollBar)
    rtView.width -= m_fScorllBarWidth;

  bool bMultiCol =
      !!(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_MultiColumn);
  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_ListItem* pItem = GetItem(this, i);
    if (!pItem)
      continue;

    CFX_RectF rtItem;
    GetItemRect(this, pItem, rtItem);
    rtItem.Offset(m_rtConent.left - fPosX, m_rtConent.top - fPosY);
    if (rtItem.bottom() < m_rtConent.top)
      continue;
    if (rtItem.top >= m_rtConent.bottom())
      break;
    if (bMultiCol && rtItem.left > m_rtConent.right())
      break;

    if (!(GetStylesEx() & FWL_STYLEEXT_LTB_OwnerDraw))
      DrawItem(pGraphics, pTheme, pItem, i, rtItem, pMatrix);
  }
}

void CFWL_ListBox::DrawItem(CFX_Graphics* pGraphics,
                            IFWL_ThemeProvider* pTheme,
                            CFWL_ListItem* pItem,
                            int32_t Index,
                            const CFX_RectF& rtItem,
                            const CFX_Matrix* pMatrix) {
  uint32_t dwItemStyles = GetItemStyles(this, pItem);
  uint32_t dwPartStates = CFWL_PartState_Normal;
  if (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)
    dwPartStates = CFWL_PartState_Disabled;
  else if (dwItemStyles & FWL_ITEMSTATE_LTB_Selected)
    dwPartStates = CFWL_PartState_Selected;

  if (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused &&
      dwItemStyles & FWL_ITEMSTATE_LTB_Focused) {
    dwPartStates |= CFWL_PartState_Focused;
  }

  CFWL_ThemeBackground bg_param;
  bg_param.m_pWidget = this;
  bg_param.m_iPart = CFWL_Part::ListItem;
  bg_param.m_dwStates = dwPartStates;
  bg_param.m_pGraphics = pGraphics;
  bg_param.m_matrix.Concat(*pMatrix);
  bg_param.m_rtPart = rtItem;
  bg_param.m_bMaximize = true;
  CFX_RectF rtFocus(rtItem);
  bg_param.m_pData = &rtFocus;
  if (m_pVertScrollBar && !m_pHorzScrollBar &&
      (dwPartStates & CFWL_PartState_Focused)) {
    bg_param.m_rtPart.left += 1;
    bg_param.m_rtPart.width -= (m_fScorllBarWidth + 1);
    rtFocus.Deflate(0.5, 0.5, 1 + m_fScorllBarWidth, 1);
  }
  pTheme->DrawBackground(&bg_param);

  bool bHasIcon = !!(GetStylesEx() & FWL_STYLEEXT_LTB_Icon);
  if (bHasIcon) {
    CFX_RectF rtDIB;
    CFX_DIBitmap* pDib = GetItemIcon(this, pItem);
    rtDIB.Set(rtItem.left, rtItem.top, rtItem.height, rtItem.height);
    if (pDib) {
      CFWL_ThemeBackground param;
      param.m_pWidget = this;
      param.m_iPart = CFWL_Part::Icon;
      param.m_pGraphics = pGraphics;
      param.m_matrix.Concat(*pMatrix);
      param.m_rtPart = rtDIB;
      param.m_bMaximize = true;
      param.m_pImage = pDib;
      pTheme->DrawBackground(&param);
    }
  }

  bool bHasCheck = !!(GetStylesEx() & FWL_STYLEEXT_LTB_Check);
  if (bHasCheck) {
    CFX_RectF rtCheck;
    rtCheck.Set(rtItem.left, rtItem.top, rtItem.height, rtItem.height);
    rtCheck.Deflate(2, 2, 2, 2);
    SetItemCheckRect(this, pItem, rtCheck);
    CFWL_ThemeBackground param;
    param.m_pWidget = this;
    param.m_iPart = CFWL_Part::Check;
    param.m_pGraphics = pGraphics;
    if (GetItemChecked(pItem))
      param.m_dwStates = CFWL_PartState_Checked;
    else
      param.m_dwStates = CFWL_PartState_Normal;
    param.m_matrix.Concat(*pMatrix);
    param.m_rtPart = rtCheck;
    param.m_bMaximize = true;
    pTheme->DrawBackground(&param);
  }

  CFX_WideString wsText = GetItemText(this, pItem);
  if (wsText.GetLength() <= 0)
    return;

  CFX_RectF rtText(rtItem);
  rtText.Deflate(kItemTextMargin, kItemTextMargin);
  if (bHasIcon || bHasCheck)
    rtText.Deflate(rtItem.height, 0, 0, 0);

  CFWL_ThemeText textParam;
  textParam.m_pWidget = this;
  textParam.m_iPart = CFWL_Part::ListItem;
  textParam.m_dwStates = dwPartStates;
  textParam.m_pGraphics = pGraphics;
  textParam.m_matrix.Concat(*pMatrix);
  textParam.m_rtPart = rtText;
  textParam.m_wsText = wsText;
  textParam.m_dwTTOStyles = m_dwTTOStyles;
  textParam.m_iTTOAlign = m_iTTOAligns;
  textParam.m_bMaximize = true;
  pTheme->DrawText(&textParam);
}

CFX_SizeF CFWL_ListBox::CalcSize(bool bAutoSize) {
  CFX_SizeF fs;
  if (!m_pProperties->m_pThemeProvider)
    return fs;

  GetClientRect(m_rtClient);
  m_rtConent = m_rtClient;
  CFX_RectF rtUIMargin;
  rtUIMargin.Set(0, 0, 0, 0);
  if (!m_pOuter) {
    CFX_RectF* pUIMargin = static_cast<CFX_RectF*>(
        GetThemeCapacity(CFWL_WidgetCapacity::UIMargin));
    if (pUIMargin) {
      m_rtConent.Deflate(pUIMargin->left, pUIMargin->top, pUIMargin->width,
                         pUIMargin->height);
    }
  }

  FX_FLOAT fWidth = GetMaxTextWidth();
  fWidth += 2 * kItemTextMargin;
  if (!bAutoSize) {
    FX_FLOAT fActualWidth =
        m_rtClient.width - rtUIMargin.left - rtUIMargin.width;
    fWidth = std::max(fWidth, fActualWidth);
  }

  m_fItemHeight = CalcItemHeight();
  if ((GetStylesEx() & FWL_STYLEEXT_LTB_Icon))
    fWidth += m_fItemHeight;

  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_ListItem* htem = GetItem(this, i);
    GetItemSize(fs, htem, fWidth, m_fItemHeight, bAutoSize);
  }
  if (bAutoSize)
    return fs;

  FX_FLOAT iWidth = m_rtClient.width - rtUIMargin.left - rtUIMargin.width;
  FX_FLOAT iHeight = m_rtClient.height;
  bool bShowVertScr =
      (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_ShowScrollBarAlaways) &&
      (m_pProperties->m_dwStyles & FWL_WGTSTYLE_VScroll);
  bool bShowHorzScr =
      (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_ShowScrollBarAlaways) &&
      (m_pProperties->m_dwStyles & FWL_WGTSTYLE_HScroll);
  if (!bShowVertScr && m_pProperties->m_dwStyles & FWL_WGTSTYLE_VScroll &&
      (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_MultiColumn) == 0) {
    bShowVertScr = (fs.y > iHeight);
  }
  if (!bShowHorzScr && m_pProperties->m_dwStyles & FWL_WGTSTYLE_HScroll)
    bShowHorzScr = (fs.x > iWidth);

  CFX_SizeF szRange;
  if (bShowVertScr) {
    if (!m_pVertScrollBar)
      InitVerticalScrollBar();

    CFX_RectF rtScrollBar;
    rtScrollBar.Set(m_rtClient.right() - m_fScorllBarWidth, m_rtClient.top,
                    m_fScorllBarWidth, m_rtClient.height - 1);
    if (bShowHorzScr)
      rtScrollBar.height -= m_fScorllBarWidth;

    m_pVertScrollBar->SetWidgetRect(rtScrollBar);
    szRange.x = 0, szRange.y = fs.y - m_rtConent.height;
    szRange.y = std::max(szRange.y, m_fItemHeight);

    m_pVertScrollBar->SetRange(szRange.x, szRange.y);
    m_pVertScrollBar->SetPageSize(rtScrollBar.height * 9 / 10);
    m_pVertScrollBar->SetStepSize(m_fItemHeight);

    FX_FLOAT fPos =
        std::min(std::max(m_pVertScrollBar->GetPos(), 0.f), szRange.y);
    m_pVertScrollBar->SetPos(fPos);
    m_pVertScrollBar->SetTrackPos(fPos);
    if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_ShowScrollBarFocus) ==
            0 ||
        (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused)) {
      m_pVertScrollBar->RemoveStates(FWL_WGTSTATE_Invisible);
    }
    m_pVertScrollBar->Update();
  } else if (m_pVertScrollBar) {
    m_pVertScrollBar->SetPos(0);
    m_pVertScrollBar->SetTrackPos(0);
    m_pVertScrollBar->SetStates(FWL_WGTSTATE_Invisible);
  }
  if (bShowHorzScr) {
    if (!m_pHorzScrollBar)
      InitHorizontalScrollBar();

    CFX_RectF rtScrollBar;
    rtScrollBar.Set(m_rtClient.left, m_rtClient.bottom() - m_fScorllBarWidth,
                    m_rtClient.width, m_fScorllBarWidth);
    if (bShowVertScr)
      rtScrollBar.width -= m_fScorllBarWidth;

    m_pHorzScrollBar->SetWidgetRect(rtScrollBar);
    szRange.x = 0, szRange.y = fs.x - rtScrollBar.width;
    m_pHorzScrollBar->SetRange(szRange.x, szRange.y);
    m_pHorzScrollBar->SetPageSize(fWidth * 9 / 10);
    m_pHorzScrollBar->SetStepSize(fWidth / 10);

    FX_FLOAT fPos =
        std::min(std::max(m_pHorzScrollBar->GetPos(), 0.f), szRange.y);
    m_pHorzScrollBar->SetPos(fPos);
    m_pHorzScrollBar->SetTrackPos(fPos);
    if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_ShowScrollBarFocus) ==
            0 ||
        (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused)) {
      m_pHorzScrollBar->RemoveStates(FWL_WGTSTATE_Invisible);
    }
    m_pHorzScrollBar->Update();
  } else if (m_pHorzScrollBar) {
    m_pHorzScrollBar->SetPos(0);
    m_pHorzScrollBar->SetTrackPos(0);
    m_pHorzScrollBar->SetStates(FWL_WGTSTATE_Invisible);
  }
  if (bShowVertScr && bShowHorzScr) {
    m_rtStatic.Set(m_rtClient.right() - m_fScorllBarWidth,
                   m_rtClient.bottom() - m_fScorllBarWidth, m_fScorllBarWidth,
                   m_fScorllBarWidth);
  }
  return fs;
}

void CFWL_ListBox::GetItemSize(CFX_SizeF& size,
                               CFWL_ListItem* pItem,
                               FX_FLOAT fWidth,
                               FX_FLOAT fItemHeight,
                               bool bAutoSize) {
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_MultiColumn)
    return;

  if (!bAutoSize) {
    CFX_RectF rtItem;
    rtItem.Set(0, size.y, fWidth, fItemHeight);
    SetItemRect(this, pItem, rtItem);
  }
  size.x = fWidth;
  size.y += fItemHeight;
}

FX_FLOAT CFWL_ListBox::GetMaxTextWidth() {
  FX_FLOAT fRet = 0.0f;
  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_ListItem* pItem = GetItem(this, i);
    if (!pItem)
      continue;

    CFX_WideString wsText = GetItemText(this, pItem);
    CFX_SizeF sz = CalcTextSize(wsText, m_pProperties->m_pThemeProvider, false);
    fRet = std::max(fRet, sz.x);
  }
  return fRet;
}

FX_FLOAT CFWL_ListBox::GetScrollWidth() {
  FX_FLOAT* pfWidth = static_cast<FX_FLOAT*>(
      GetThemeCapacity(CFWL_WidgetCapacity::ScrollBarWidth));
  if (!pfWidth)
    return 0;
  return *pfWidth;
}

FX_FLOAT CFWL_ListBox::CalcItemHeight() {
  FX_FLOAT* pfFont =
      static_cast<FX_FLOAT*>(GetThemeCapacity(CFWL_WidgetCapacity::FontSize));
  if (!pfFont)
    return 20;
  return *pfFont + 2 * kItemTextMargin;
}

void CFWL_ListBox::InitVerticalScrollBar() {
  if (m_pVertScrollBar)
    return;

  auto prop = pdfium::MakeUnique<CFWL_WidgetProperties>();
  prop->m_dwStyleExes = FWL_STYLEEXT_SCB_Vert;
  prop->m_dwStates = FWL_WGTSTATE_Invisible;
  prop->m_pParent = this;
  prop->m_pThemeProvider = m_pScrollBarTP;
  m_pVertScrollBar =
      pdfium::MakeUnique<CFWL_ScrollBar>(m_pOwnerApp, std::move(prop), this);
}

void CFWL_ListBox::InitHorizontalScrollBar() {
  if (m_pHorzScrollBar)
    return;

  auto prop = pdfium::MakeUnique<CFWL_WidgetProperties>();
  prop->m_dwStyleExes = FWL_STYLEEXT_SCB_Horz;
  prop->m_dwStates = FWL_WGTSTATE_Invisible;
  prop->m_pParent = this;
  prop->m_pThemeProvider = m_pScrollBarTP;
  m_pHorzScrollBar =
      pdfium::MakeUnique<CFWL_ScrollBar>(m_pOwnerApp, std::move(prop), this);
}

bool CFWL_ListBox::IsShowScrollBar(bool bVert) {
  CFWL_ScrollBar* pScrollbar =
      bVert ? m_pVertScrollBar.get() : m_pHorzScrollBar.get();
  if (!pScrollbar || (pScrollbar->GetStates() & FWL_WGTSTATE_Invisible))
    return false;
  return !(m_pProperties->m_dwStyleExes &
           FWL_STYLEEXT_LTB_ShowScrollBarFocus) ||
         (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused);
}

void CFWL_ListBox::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return;
  if (!IsEnabled())
    return;

  switch (pMessage->GetType()) {
    case CFWL_Message::Type::SetFocus:
      OnFocusChanged(pMessage, true);
      break;
    case CFWL_Message::Type::KillFocus:
      OnFocusChanged(pMessage, false);
      break;
    case CFWL_Message::Type::Mouse: {
      CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
      switch (pMsg->m_dwCmd) {
        case FWL_MouseCommand::LeftButtonDown:
          OnLButtonDown(pMsg);
          break;
        case FWL_MouseCommand::LeftButtonUp:
          OnLButtonUp(pMsg);
          break;
        default:
          break;
      }
      break;
    }
    case CFWL_Message::Type::MouseWheel:
      OnMouseWheel(static_cast<CFWL_MsgMouseWheel*>(pMessage));
      break;
    case CFWL_Message::Type::Key: {
      CFWL_MsgKey* pMsg = static_cast<CFWL_MsgKey*>(pMessage);
      if (pMsg->m_dwCmd == FWL_KeyCommand::KeyDown)
        OnKeyDown(pMsg);
      break;
    }
    default:
      break;
  }
  CFWL_Widget::OnProcessMessage(pMessage);
}

void CFWL_ListBox::OnProcessEvent(CFWL_Event* pEvent) {
  if (!pEvent)
    return;
  if (pEvent->GetType() != CFWL_Event::Type::Scroll)
    return;

  CFWL_Widget* pSrcTarget = pEvent->m_pSrcTarget;
  if ((pSrcTarget == m_pVertScrollBar.get() && m_pVertScrollBar) ||
      (pSrcTarget == m_pHorzScrollBar.get() && m_pHorzScrollBar)) {
    CFWL_EvtScroll* pScrollEvent = static_cast<CFWL_EvtScroll*>(pEvent);
    OnScroll(static_cast<CFWL_ScrollBar*>(pSrcTarget),
             pScrollEvent->m_iScrollCode, pScrollEvent->m_fPos);
  }
}

void CFWL_ListBox::OnDrawWidget(CFX_Graphics* pGraphics,
                                const CFX_Matrix* pMatrix) {
  DrawWidget(pGraphics, pMatrix);
}

void CFWL_ListBox::OnFocusChanged(CFWL_Message* pMsg, bool bSet) {
  if (GetStylesEx() & FWL_STYLEEXT_LTB_ShowScrollBarFocus) {
    if (m_pVertScrollBar) {
      if (bSet)
        m_pVertScrollBar->RemoveStates(FWL_WGTSTATE_Invisible);
      else
        m_pVertScrollBar->SetStates(FWL_WGTSTATE_Invisible);
    }
    if (m_pHorzScrollBar) {
      if (bSet)
        m_pHorzScrollBar->RemoveStates(FWL_WGTSTATE_Invisible);
      else
        m_pHorzScrollBar->SetStates(FWL_WGTSTATE_Invisible);
    }
  }
  if (bSet)
    m_pProperties->m_dwStates |= (FWL_WGTSTATE_Focused);
  else
    m_pProperties->m_dwStates &= ~(FWL_WGTSTATE_Focused);

  Repaint(&m_rtClient);
}

void CFWL_ListBox::OnLButtonDown(CFWL_MsgMouse* pMsg) {
  m_bLButtonDown = true;
  if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0)
    SetFocus(true);

  CFWL_ListItem* pItem = GetItemAtPoint(pMsg->m_fx, pMsg->m_fy);
  if (!pItem)
    return;

  if (IsMultiSelection()) {
    if (pMsg->m_dwFlags & FWL_KEYFLAG_Ctrl) {
      bool bSelected = IsItemSelected(pItem);
      SetSelectionDirect(pItem, !bSelected);
      m_hAnchor = pItem;
    } else if (pMsg->m_dwFlags & FWL_KEYFLAG_Shift) {
      if (m_hAnchor)
        SetSelection(m_hAnchor, pItem, true);
      else
        SetSelectionDirect(pItem, true);
    } else {
      SetSelection(pItem, pItem, true);
      m_hAnchor = pItem;
    }
  } else {
    SetSelection(pItem, pItem, true);
  }
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_Check) {
    CFWL_ListItem* hSelectedItem = GetItemAtPoint(pMsg->m_fx, pMsg->m_fy);
    CFX_RectF rtCheck;
    GetItemCheckRectInternal(hSelectedItem, rtCheck);
    bool bChecked = GetItemChecked(pItem);
    if (rtCheck.Contains(pMsg->m_fx, pMsg->m_fy)) {
      SetItemChecked(pItem, !bChecked);
      Update();
    }
  }
  SetFocusItem(pItem);
  ScrollToVisible(pItem);
  SetGrab(true);
  Repaint(&m_rtClient);
}

void CFWL_ListBox::OnLButtonUp(CFWL_MsgMouse* pMsg) {
  if (!m_bLButtonDown)
    return;

  m_bLButtonDown = false;
  SetGrab(false);
}

void CFWL_ListBox::OnMouseWheel(CFWL_MsgMouseWheel* pMsg) {
  if (IsShowScrollBar(true))
    m_pVertScrollBar->GetDelegate()->OnProcessMessage(pMsg);
}

void CFWL_ListBox::OnKeyDown(CFWL_MsgKey* pMsg) {
  uint32_t dwKeyCode = pMsg->m_dwKeyCode;
  switch (dwKeyCode) {
    case FWL_VKEY_Tab:
    case FWL_VKEY_Up:
    case FWL_VKEY_Down:
    case FWL_VKEY_Home:
    case FWL_VKEY_End: {
      CFWL_ListItem* pItem = GetFocusedItem();
      pItem = GetListItem(pItem, dwKeyCode);
      bool bShift = !!(pMsg->m_dwFlags & FWL_KEYFLAG_Shift);
      bool bCtrl = !!(pMsg->m_dwFlags & FWL_KEYFLAG_Ctrl);
      OnVK(pItem, bShift, bCtrl);
      break;
    }
    default:
      break;
  }
}

void CFWL_ListBox::OnVK(CFWL_ListItem* pItem, bool bShift, bool bCtrl) {
  if (!pItem)
    return;

  if (IsMultiSelection()) {
    if (bCtrl) {
      // Do nothing.
    } else if (bShift) {
      if (m_hAnchor)
        SetSelection(m_hAnchor, pItem, true);
      else
        SetSelectionDirect(pItem, true);
    } else {
      SetSelection(pItem, pItem, true);
      m_hAnchor = pItem;
    }
  } else {
    SetSelection(pItem, pItem, true);
  }

  SetFocusItem(pItem);
  ScrollToVisible(pItem);

  CFX_RectF rtInvalidate;
  rtInvalidate.Set(0, 0, m_pProperties->m_rtWidget.width,
                   m_pProperties->m_rtWidget.height);
  Repaint(&rtInvalidate);
}

bool CFWL_ListBox::OnScroll(CFWL_ScrollBar* pScrollBar,
                            CFWL_EvtScroll::Code dwCode,
                            FX_FLOAT fPos) {
  CFX_SizeF fs;
  pScrollBar->GetRange(&fs.x, &fs.y);
  FX_FLOAT iCurPos = pScrollBar->GetPos();
  FX_FLOAT fStep = pScrollBar->GetStepSize();
  switch (dwCode) {
    case CFWL_EvtScroll::Code::Min: {
      fPos = fs.x;
      break;
    }
    case CFWL_EvtScroll::Code::Max: {
      fPos = fs.y;
      break;
    }
    case CFWL_EvtScroll::Code::StepBackward: {
      fPos -= fStep;
      if (fPos < fs.x + fStep / 2)
        fPos = fs.x;
      break;
    }
    case CFWL_EvtScroll::Code::StepForward: {
      fPos += fStep;
      if (fPos > fs.y - fStep / 2)
        fPos = fs.y;
      break;
    }
    case CFWL_EvtScroll::Code::PageBackward: {
      fPos -= pScrollBar->GetPageSize();
      if (fPos < fs.x)
        fPos = fs.x;
      break;
    }
    case CFWL_EvtScroll::Code::PageForward: {
      fPos += pScrollBar->GetPageSize();
      if (fPos > fs.y)
        fPos = fs.y;
      break;
    }
    case CFWL_EvtScroll::Code::Pos:
    case CFWL_EvtScroll::Code::TrackPos:
    case CFWL_EvtScroll::Code::None:
      break;
    case CFWL_EvtScroll::Code::EndScroll:
      return false;
  }
  if (iCurPos != fPos) {
    pScrollBar->SetPos(fPos);
    pScrollBar->SetTrackPos(fPos);
    Repaint(&m_rtClient);
  }
  return true;
}

CFX_WideString CFWL_ListBox::GetItemText(CFWL_Widget* pWidget,
                                         CFWL_ListItem* pItem) {
  return pItem ? static_cast<CFWL_ListItem*>(pItem)->m_wsText : L"";
}

int32_t CFWL_ListBox::CountItems(const CFWL_Widget* pWidget) const {
  return pdfium::CollectionSize<int32_t>(m_ItemArray);
}

CFWL_ListItem* CFWL_ListBox::GetItem(const CFWL_Widget* pWidget,
                                     int32_t nIndex) const {
  if (nIndex < 0 || nIndex >= CountItems(pWidget))
    return nullptr;
  return m_ItemArray[nIndex].get();
}

int32_t CFWL_ListBox::GetItemIndex(CFWL_Widget* pWidget, CFWL_ListItem* pItem) {
  auto it = std::find_if(
      m_ItemArray.begin(), m_ItemArray.end(),
      [pItem](const std::unique_ptr<CFWL_ListItem>& candidate) {
        return candidate.get() == static_cast<CFWL_ListItem*>(pItem);
      });
  return it != m_ItemArray.end() ? it - m_ItemArray.begin() : -1;
}

uint32_t CFWL_ListBox::GetItemStyles(CFWL_Widget* pWidget,
                                     CFWL_ListItem* pItem) {
  return pItem ? static_cast<CFWL_ListItem*>(pItem)->m_dwStates : 0;
}

void CFWL_ListBox::GetItemRect(CFWL_Widget* pWidget,
                               CFWL_ListItem* pItem,
                               CFX_RectF& rtItem) {
  if (pItem)
    rtItem = static_cast<CFWL_ListItem*>(pItem)->m_rtItem;
}

void* CFWL_ListBox::GetItemData(CFWL_Widget* pWidget, CFWL_ListItem* pItem) {
  return pItem ? static_cast<CFWL_ListItem*>(pItem)->m_pData : nullptr;
}

void CFWL_ListBox::SetItemStyles(CFWL_Widget* pWidget,
                                 CFWL_ListItem* pItem,
                                 uint32_t dwStyle) {
  if (pItem)
    static_cast<CFWL_ListItem*>(pItem)->m_dwStates = dwStyle;
}

void CFWL_ListBox::SetItemRect(CFWL_Widget* pWidget,
                               CFWL_ListItem* pItem,
                               const CFX_RectF& rtItem) {
  if (pItem)
    static_cast<CFWL_ListItem*>(pItem)->m_rtItem = rtItem;
}

CFX_DIBitmap* CFWL_ListBox::GetItemIcon(CFWL_Widget* pWidget,
                                        CFWL_ListItem* pItem) {
  return static_cast<CFWL_ListItem*>(pItem)->m_pDIB;
}

void CFWL_ListBox::GetItemCheckRect(CFWL_Widget* pWidget,
                                    CFWL_ListItem* pItem,
                                    CFX_RectF& rtCheck) {
  rtCheck = static_cast<CFWL_ListItem*>(pItem)->m_rtCheckBox;
}

void CFWL_ListBox::SetItemCheckRect(CFWL_Widget* pWidget,
                                    CFWL_ListItem* pItem,
                                    const CFX_RectF& rtCheck) {
  static_cast<CFWL_ListItem*>(pItem)->m_rtCheckBox = rtCheck;
}

uint32_t CFWL_ListBox::GetItemCheckState(CFWL_Widget* pWidget,
                                         CFWL_ListItem* pItem) {
  return static_cast<CFWL_ListItem*>(pItem)->m_dwCheckState;
}

void CFWL_ListBox::SetItemCheckState(CFWL_Widget* pWidget,
                                     CFWL_ListItem* pItem,
                                     uint32_t dwCheckState) {
  static_cast<CFWL_ListItem*>(pItem)->m_dwCheckState = dwCheckState;
}

CFWL_ListItem* CFWL_ListBox::AddString(const CFX_WideStringC& wsAdd) {
  auto pItem = pdfium::MakeUnique<CFWL_ListItem>();
  pItem->m_dwStates = 0;
  pItem->m_wsText = wsAdd;
  pItem->m_dwStates = 0;
  m_ItemArray.push_back(std::move(pItem));
  return m_ItemArray.back().get();
}

bool CFWL_ListBox::RemoveAt(int32_t iIndex) {
  if (iIndex < 0 || static_cast<size_t>(iIndex) >= m_ItemArray.size())
    return false;

  m_ItemArray.erase(m_ItemArray.begin() + iIndex);
  return true;
}

bool CFWL_ListBox::DeleteString(CFWL_ListItem* pItem) {
  int32_t nIndex = GetItemIndex(this, pItem);
  if (nIndex < 0 || static_cast<size_t>(nIndex) >= m_ItemArray.size())
    return false;

  int32_t iSel = nIndex + 1;
  if (iSel >= CountItems(this))
    iSel = nIndex - 1;
  if (iSel >= 0) {
    CFWL_ListItem* pSel = static_cast<CFWL_ListItem*>(GetItem(this, iSel));
    pSel->m_dwStates |= FWL_ITEMSTATE_LTB_Selected;
  }
  m_ItemArray.erase(m_ItemArray.begin() + nIndex);
  return true;
}

void CFWL_ListBox::DeleteAll() {
  m_ItemArray.clear();
}

uint32_t CFWL_ListBox::GetItemStates(CFWL_ListItem* pItem) {
  if (!pItem)
    return 0;
  return pItem->m_dwStates | pItem->m_dwCheckState;
}

