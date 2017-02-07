// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_widgetmgr.h"

#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_form.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fxfa/app/xfa_fwladapter.h"
#include "xfa/fxfa/xfa_ffapp.h"

namespace {

const int kNeedRepaintHitPoints = 12;
const int kNeedRepaintHitPiece = 3;

struct FWL_NEEDREPAINTHITDATA {
  CFX_PointF hitPoint;
  bool bNotNeedRepaint;
  bool bNotContainByDirty;
};

}  // namespace

CFWL_WidgetMgr::CFWL_WidgetMgr(CXFA_FFApp* pAdapterNative)
    : m_dwCapability(0), m_pAdapter(pAdapterNative->GetWidgetMgr(this)) {
  ASSERT(m_pAdapter);
  m_mapWidgetItem[nullptr] = pdfium::MakeUnique<Item>();
#if (_FX_OS_ == _FX_WIN32_DESKTOP_) || (_FX_OS_ == _FX_WIN64_)
  m_rtScreen.Reset();
#endif
}

CFWL_WidgetMgr::~CFWL_WidgetMgr() {}

CFWL_Widget* CFWL_WidgetMgr::GetParentWidget(CFWL_Widget* pWidget) const {
  Item* pItem = GetWidgetMgrItem(pWidget);
  return pItem && pItem->pParent ? pItem->pParent->pWidget : nullptr;
}

CFWL_Widget* CFWL_WidgetMgr::GetOwnerWidget(CFWL_Widget* pWidget) const {
  Item* pItem = GetWidgetMgrItem(pWidget);
  return pItem && pItem->pOwner ? pItem->pOwner->pWidget : nullptr;
}

CFWL_Widget* CFWL_WidgetMgr::GetFirstSiblingWidget(CFWL_Widget* pWidget) const {
  Item* pItem = GetWidgetMgrItem(pWidget);
  if (!pItem)
    return nullptr;

  pItem = pItem->pPrevious;
  while (pItem && pItem->pPrevious)
    pItem = pItem->pPrevious;
  return pItem ? pItem->pWidget : nullptr;
}

CFWL_Widget* CFWL_WidgetMgr::GetPriorSiblingWidget(CFWL_Widget* pWidget) const {
  Item* pItem = GetWidgetMgrItem(pWidget);
  return pItem && pItem->pPrevious ? pItem->pPrevious->pWidget : nullptr;
}

CFWL_Widget* CFWL_WidgetMgr::GetNextSiblingWidget(CFWL_Widget* pWidget) const {
  Item* pItem = GetWidgetMgrItem(pWidget);
  return pItem && pItem->pNext ? pItem->pNext->pWidget : nullptr;
}

CFWL_Widget* CFWL_WidgetMgr::GetFirstChildWidget(CFWL_Widget* pWidget) const {
  Item* pItem = GetWidgetMgrItem(pWidget);
  return pItem && pItem->pChild ? pItem->pChild->pWidget : nullptr;
}

CFWL_Widget* CFWL_WidgetMgr::GetLastChildWidget(CFWL_Widget* pWidget) const {
  Item* pItem = GetWidgetMgrItem(pWidget);
  if (!pItem)
    return nullptr;

  pItem = pItem->pChild;
  while (pItem && pItem->pNext)
    pItem = pItem->pNext;
  return pItem ? pItem->pWidget : nullptr;
}

CFWL_Widget* CFWL_WidgetMgr::GetSystemFormWidget(CFWL_Widget* pWidget) const {
  Item* pItem = GetWidgetMgrItem(pWidget);
  while (pItem) {
    if (IsAbleNative(pItem->pWidget))
      return pItem->pWidget;
    pItem = pItem->pParent;
  }
  return nullptr;
}

void CFWL_WidgetMgr::AppendWidget(CFWL_Widget* pWidget) {
  Item* pItem = GetWidgetMgrItem(pWidget);
  if (!pItem)
    return;
  if (!pItem->pParent)
    return;

  Item* pChild = pItem->pParent->pChild;
  int32_t i = 0;
  while (pChild) {
    if (pChild == pItem) {
      if (pChild->pPrevious)
        pChild->pPrevious->pNext = pChild->pNext;
      if (pChild->pNext)
        pChild->pNext->pPrevious = pChild->pPrevious;
      if (pItem->pParent->pChild == pItem)
        pItem->pParent->pChild = pItem->pNext;

      pItem->pNext = nullptr;
      pItem->pPrevious = nullptr;
      break;
    }
    if (!pChild->pNext)
      break;

    pChild = pChild->pNext;
    ++i;
  }

  pChild = pItem->pParent->pChild;
  if (pChild) {
    while (pChild->pNext)
      pChild = pChild->pNext;

    pChild->pNext = pItem;
    pItem->pPrevious = pChild;
  } else {
    pItem->pParent->pChild = pItem;
    pItem->pPrevious = nullptr;
  }
  pItem->pNext = nullptr;
}

void CFWL_WidgetMgr::RepaintWidget(CFWL_Widget* pWidget,
                                   const CFX_RectF& rect) {
  if (!m_pAdapter)
    return;

  CFWL_Widget* pNative = pWidget;
  CFX_RectF transformedRect = rect;
  if (IsFormDisabled()) {
    CFWL_Widget* pOuter = pWidget->GetOuter();
    while (pOuter) {
      CFX_RectF rtTemp = pNative->GetWidgetRect();
      transformedRect.left += rtTemp.left;
      transformedRect.top += rtTemp.top;
      pNative = pOuter;
      pOuter = pOuter->GetOuter();
    }
  } else if (!IsAbleNative(pWidget)) {
    pNative = GetSystemFormWidget(pWidget);
    if (!pNative)
      return;

    pWidget->TransformTo(pNative, transformedRect.left, transformedRect.top);
  }
  AddRedrawCounts(pNative);
  m_pAdapter->RepaintWidget(pNative);
}

void CFWL_WidgetMgr::InsertWidget(CFWL_Widget* pParent, CFWL_Widget* pChild) {
  Item* pParentItem = GetWidgetMgrItem(pParent);
  if (!pParentItem) {
    auto item = pdfium::MakeUnique<Item>(pParent);
    pParentItem = item.get();
    m_mapWidgetItem[pParent] = std::move(item);

    pParentItem->pParent = GetWidgetMgrItem(nullptr);
    AppendWidget(pParent);
  }

  Item* pItem = GetWidgetMgrItem(pChild);
  if (!pItem) {
    auto item = pdfium::MakeUnique<Item>(pChild);
    pItem = item.get();
    m_mapWidgetItem[pChild] = std::move(item);
  }
  if (pItem->pParent && pItem->pParent != pParentItem) {
    if (pItem->pPrevious)
      pItem->pPrevious->pNext = pItem->pNext;
    if (pItem->pNext)
      pItem->pNext->pPrevious = pItem->pPrevious;
    if (pItem->pParent->pChild == pItem)
      pItem->pParent->pChild = pItem->pNext;
  }
  pItem->pParent = pParentItem;
  AppendWidget(pChild);
}

void CFWL_WidgetMgr::RemoveWidget(CFWL_Widget* pWidget) {
  Item* pItem = GetWidgetMgrItem(pWidget);
  if (!pItem)
    return;
  if (pItem->pPrevious)
    pItem->pPrevious->pNext = pItem->pNext;
  if (pItem->pNext)
    pItem->pNext->pPrevious = pItem->pPrevious;
  if (pItem->pParent && pItem->pParent->pChild == pItem)
    pItem->pParent->pChild = pItem->pNext;

  Item* pChild = pItem->pChild;
  while (pChild) {
    Item* pNext = pChild->pNext;
    RemoveWidget(pChild->pWidget);
    pChild = pNext;
  }
  m_mapWidgetItem.erase(pWidget);
}

void CFWL_WidgetMgr::SetOwner(CFWL_Widget* pOwner, CFWL_Widget* pOwned) {
  Item* pParentItem = GetWidgetMgrItem(pOwner);
  if (!pParentItem) {
    auto item = pdfium::MakeUnique<Item>(pOwner);
    pParentItem = item.get();
    m_mapWidgetItem[pOwner] = std::move(item);

    pParentItem->pParent = GetWidgetMgrItem(nullptr);
    AppendWidget(pOwner);
  }

  Item* pItem = GetWidgetMgrItem(pOwned);
  if (!pItem) {
    auto item = pdfium::MakeUnique<Item>(pOwned);
    pItem = item.get();
    m_mapWidgetItem[pOwned] = std::move(item);
  }
  pItem->pOwner = pParentItem;
}
void CFWL_WidgetMgr::SetParent(CFWL_Widget* pParent, CFWL_Widget* pChild) {
  Item* pParentItem = GetWidgetMgrItem(pParent);
  Item* pItem = GetWidgetMgrItem(pChild);
  if (!pItem)
    return;
  if (pItem->pParent && pItem->pParent != pParentItem) {
    if (pItem->pPrevious)
      pItem->pPrevious->pNext = pItem->pNext;
    if (pItem->pNext)
      pItem->pNext->pPrevious = pItem->pPrevious;
    if (pItem->pParent->pChild == pItem)
      pItem->pParent->pChild = pItem->pNext;

    pItem->pNext = nullptr;
    pItem->pPrevious = nullptr;
  }
  pItem->pParent = pParentItem;
  AppendWidget(pChild);
}

CFWL_Widget* CFWL_WidgetMgr::GetWidgetAtPoint(CFWL_Widget* parent,
                                              FX_FLOAT x,
                                              FX_FLOAT y) {
  if (!parent)
    return nullptr;

  FX_FLOAT x1;
  FX_FLOAT y1;
  CFWL_Widget* child = GetLastChildWidget(parent);
  while (child) {
    if ((child->GetStates() & FWL_WGTSTATE_Invisible) == 0) {
      x1 = x;
      y1 = y;
      CFX_Matrix m;
      m.SetIdentity();

      CFX_Matrix matrixOnParent;
      m.SetReverse(matrixOnParent);
      m.TransformPoint(x1, y1);
      CFX_RectF bounds = child->GetWidgetRect();
      if (bounds.Contains(x1, y1)) {
        x1 -= bounds.left;
        y1 -= bounds.top;
        return GetWidgetAtPoint(child, x1, y1);
      }
    }
    child = GetPriorSiblingWidget(child);
  }
  return parent;
}

CFWL_Widget* CFWL_WidgetMgr::NextTab(CFWL_Widget* parent,
                                     CFWL_Widget* focus,
                                     bool& bFind) {
  CFWL_WidgetMgr* pMgr = parent->GetOwnerApp()->GetWidgetMgr();
  CFWL_Widget* child = pMgr->GetFirstChildWidget(parent);
  while (child) {
    if (focus == child)
      bFind = true;

    CFWL_Widget* bRet = NextTab(child, focus, bFind);
    if (bRet)
      return bRet;

    child = pMgr->GetNextSiblingWidget(child);
  }
  return nullptr;
}

int32_t CFWL_WidgetMgr::CountRadioButtonGroup(CFWL_Widget* pFirst) const {
  int32_t iRet = 0;
  CFWL_Widget* pChild = pFirst;
  while (pChild) {
    pChild = GetNextSiblingWidget(pChild);
    ++iRet;
  }
  return iRet;
}

CFWL_Widget* CFWL_WidgetMgr::GetRadioButtonGroupHeader(
    CFWL_Widget* pRadioButton) const {
  CFWL_Widget* pNext = pRadioButton;
  if (pNext && (pNext->GetStyles() & FWL_WGTSTYLE_Group))
    return pNext;
  return nullptr;
}

std::vector<CFWL_Widget*> CFWL_WidgetMgr::GetSameGroupRadioButton(
    CFWL_Widget* pRadioButton) const {
  CFWL_Widget* pFirst = GetFirstSiblingWidget(pRadioButton);
  if (!pFirst)
    pFirst = pRadioButton;

  if (CountRadioButtonGroup(pFirst) < 2)
    return std::vector<CFWL_Widget*>();

  std::vector<CFWL_Widget*> group;
  group.push_back(GetRadioButtonGroupHeader(pRadioButton));
  return group;
}

CFWL_Widget* CFWL_WidgetMgr::GetDefaultButton(CFWL_Widget* pParent) const {
  if ((pParent->GetClassID() == FWL_Type::PushButton) &&
      (pParent->GetStates() & (1 << (FWL_WGTSTATE_MAX + 2)))) {
    return pParent;
  }

  CFWL_Widget* child =
      pParent->GetOwnerApp()->GetWidgetMgr()->GetFirstChildWidget(pParent);
  while (child) {
    if ((child->GetClassID() == FWL_Type::PushButton) &&
        (child->GetStates() & (1 << (FWL_WGTSTATE_MAX + 2)))) {
      return child;
    }
    if (CFWL_Widget* find = GetDefaultButton(child))
      return find;

    child = child->GetOwnerApp()->GetWidgetMgr()->GetNextSiblingWidget(child);
  }
  return nullptr;
}

void CFWL_WidgetMgr::AddRedrawCounts(CFWL_Widget* pWidget) {
  GetWidgetMgrItem(pWidget)->iRedrawCounter++;
}

void CFWL_WidgetMgr::ResetRedrawCounts(CFWL_Widget* pWidget) {
  GetWidgetMgrItem(pWidget)->iRedrawCounter = 0;
}

CFWL_WidgetMgr::Item* CFWL_WidgetMgr::GetWidgetMgrItem(
    CFWL_Widget* pWidget) const {
  auto it = m_mapWidgetItem.find(pWidget);
  return it != m_mapWidgetItem.end() ? static_cast<Item*>(it->second.get())
                                     : nullptr;
}

bool CFWL_WidgetMgr::IsAbleNative(CFWL_Widget* pWidget) const {
  if (!pWidget)
    return false;
  if (!pWidget->IsInstance(FWL_CLASS_Form))
    return false;

  uint32_t dwStyles = pWidget->GetStyles();
  return ((dwStyles & FWL_WGTSTYLE_WindowTypeMask) ==
          FWL_WGTSTYLE_OverLapper) ||
         (dwStyles & FWL_WGTSTYLE_Popup);
}

void CFWL_WidgetMgr::GetAdapterPopupPos(CFWL_Widget* pWidget,
                                        FX_FLOAT fMinHeight,
                                        FX_FLOAT fMaxHeight,
                                        const CFX_RectF& rtAnchor,
                                        CFX_RectF& rtPopup) const {
  m_pAdapter->GetPopupPos(pWidget, fMinHeight, fMaxHeight, rtAnchor, rtPopup);
}

void CFWL_WidgetMgr::OnSetCapability(uint32_t dwCapability) {
  m_dwCapability = dwCapability;
}

void CFWL_WidgetMgr::OnProcessMessageToForm(CFWL_Message* pMessage) {
  if (!pMessage)
    return;
  if (!pMessage->m_pDstTarget)
    return;

  CFWL_Widget* pDstWidget = pMessage->m_pDstTarget;
  const CFWL_App* pApp = pDstWidget->GetOwnerApp();
  if (!pApp)
    return;

  CFWL_NoteDriver* pNoteDriver =
      static_cast<CFWL_NoteDriver*>(pApp->GetNoteDriver());
  if (!pNoteDriver)
    return;

  if (IsFormDisabled())
    pNoteDriver->ProcessMessage(pMessage->Clone());
  else
    pNoteDriver->QueueMessage(pMessage->Clone());

#if (_FX_OS_ == _FX_MACOSX_)
  CFWL_NoteLoop* pTopLoop = pNoteDriver->GetTopLoop();
  if (pTopLoop)
    pNoteDriver->UnqueueMessageAndProcess(pTopLoop);
#endif
}

void CFWL_WidgetMgr::OnDrawWidget(CFWL_Widget* pWidget,
                                  CFX_Graphics* pGraphics,
                                  const CFX_Matrix* pMatrix) {
  if (!pWidget || !pGraphics)
    return;

  CFX_RectF clipCopy = pWidget->GetWidgetRect();
  clipCopy.Set(0, 0, clipCopy.width, clipCopy.height);

  CFX_RectF clipBounds;

#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN64_ || \
    _FX_OS_ == _FX_LINUX_DESKTOP_ || _FX_OS_ == _FX_ANDROID_
  pWidget->GetDelegate()->OnDrawWidget(pGraphics, pMatrix);
  pGraphics->GetClipRect(clipBounds);
  clipCopy = clipBounds;
#elif _FX_OS_ == _FX_MACOSX_
  if (IsFormDisabled()) {
    pWidget->GetDelegate()->OnDrawWidget(pGraphics, pMatrix);
    pGraphics->GetClipRect(clipBounds);
    clipCopy = clipBounds;
  } else {
    clipBounds.Set(pMatrix->a, pMatrix->b, pMatrix->c, pMatrix->d);
    const_cast<CFX_Matrix*>(pMatrix)->SetIdentity();  // FIXME: const cast.
    pWidget->GetDelegate()->OnDrawWidget(pGraphics, pMatrix);
  }
#endif  // _FX_OS_ == _FX_MACOSX_

  if (!IsFormDisabled())
    clipBounds.Intersect(pWidget->GetClientRect());
  if (!clipBounds.IsEmpty())
    DrawChild(pWidget, clipBounds, pGraphics, pMatrix);

  GetWidgetMgrItem(pWidget)->iRedrawCounter = 0;
  ResetRedrawCounts(pWidget);
}

void CFWL_WidgetMgr::DrawChild(CFWL_Widget* parent,
                               const CFX_RectF& rtClip,
                               CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix) {
  if (!parent)
    return;

  bool bFormDisable = IsFormDisabled();
  CFWL_Widget* pNextChild = GetFirstChildWidget(parent);
  while (pNextChild) {
    CFWL_Widget* child = pNextChild;
    pNextChild = GetNextSiblingWidget(child);
    if (child->GetStates() & FWL_WGTSTATE_Invisible)
      continue;

    CFX_RectF rtWidget = child->GetWidgetRect();
    if (rtWidget.IsEmpty())
      continue;

    CFX_Matrix widgetMatrix;
    CFX_RectF clipBounds(rtWidget);
    if (!bFormDisable)
      widgetMatrix = child->GetMatrix();
    if (pMatrix)
      widgetMatrix.Concat(*pMatrix);

    if (!bFormDisable) {
      widgetMatrix.TransformPoint(clipBounds.left, clipBounds.top);
      clipBounds.Intersect(rtClip);
      if (clipBounds.IsEmpty())
        continue;

      pGraphics->SaveGraphState();
      pGraphics->SetClipRect(clipBounds);
    }
    widgetMatrix.Translate(rtWidget.left, rtWidget.top, true);

    if (IFWL_WidgetDelegate* pDelegate = child->GetDelegate()) {
      if (IsFormDisabled() || IsNeedRepaint(child, &widgetMatrix, rtClip))
        pDelegate->OnDrawWidget(pGraphics, &widgetMatrix);
    }
    if (!bFormDisable)
      pGraphics->RestoreGraphState();

    DrawChild(child, clipBounds, pGraphics,
              bFormDisable ? &widgetMatrix : pMatrix);
    child = GetNextSiblingWidget(child);
  }
}

bool CFWL_WidgetMgr::IsNeedRepaint(CFWL_Widget* pWidget,
                                   CFX_Matrix* pMatrix,
                                   const CFX_RectF& rtDirty) {
  Item* pItem = GetWidgetMgrItem(pWidget);
  if (pItem && pItem->iRedrawCounter > 0) {
    pItem->iRedrawCounter = 0;
    return true;
  }

  CFX_RectF rtWidget = pWidget->GetWidgetRect();
  rtWidget.Set(0, 0, rtWidget.width, rtWidget.height);

  pMatrix->TransformRect(rtWidget);
  if (!rtWidget.IntersectWith(rtDirty))
    return false;

  CFWL_Widget* pChild =
      pWidget->GetOwnerApp()->GetWidgetMgr()->GetFirstChildWidget(pWidget);
  if (!pChild)
    return true;

  CFX_RectF rtChilds;
  bool bChildIntersectWithDirty = false;
  bool bOrginPtIntersectWidthChild = false;
  bool bOrginPtIntersectWidthDirty =
      rtDirty.Contains(rtWidget.left, rtWidget.top);
  static FWL_NEEDREPAINTHITDATA hitPoint[kNeedRepaintHitPoints];
  FXSYS_memset(hitPoint, 0, sizeof(hitPoint));
  FX_FLOAT fxPiece = rtWidget.width / kNeedRepaintHitPiece;
  FX_FLOAT fyPiece = rtWidget.height / kNeedRepaintHitPiece;
  hitPoint[2].hitPoint.x = hitPoint[6].hitPoint.x = rtWidget.left;
  hitPoint[0].hitPoint.x = hitPoint[3].hitPoint.x = hitPoint[7].hitPoint.x =
      hitPoint[10].hitPoint.x = fxPiece + rtWidget.left;
  hitPoint[1].hitPoint.x = hitPoint[4].hitPoint.x = hitPoint[8].hitPoint.x =
      hitPoint[11].hitPoint.x = fxPiece * 2 + rtWidget.left;
  hitPoint[5].hitPoint.x = hitPoint[9].hitPoint.x =
      rtWidget.width + rtWidget.left;
  hitPoint[0].hitPoint.y = hitPoint[1].hitPoint.y = rtWidget.top;
  hitPoint[2].hitPoint.y = hitPoint[3].hitPoint.y = hitPoint[4].hitPoint.y =
      hitPoint[5].hitPoint.y = fyPiece + rtWidget.top;
  hitPoint[6].hitPoint.y = hitPoint[7].hitPoint.y = hitPoint[8].hitPoint.y =
      hitPoint[9].hitPoint.y = fyPiece * 2 + rtWidget.top;
  hitPoint[10].hitPoint.y = hitPoint[11].hitPoint.y =
      rtWidget.height + rtWidget.top;
  do {
    CFX_RectF rect = pChild->GetWidgetRect();
    CFX_RectF r(rect.left + rtWidget.left, rect.top + rtWidget.top, rect.width,
                rect.height);
    if (r.IsEmpty())
      continue;
    if (r.Contains(rtDirty))
      return false;
    if (!bChildIntersectWithDirty && r.IntersectWith(rtDirty))
      bChildIntersectWithDirty = true;
    if (bOrginPtIntersectWidthDirty && !bOrginPtIntersectWidthChild)
      bOrginPtIntersectWidthChild = rect.Contains(0, 0);

    if (rtChilds.IsEmpty())
      rtChilds = rect;
    else if (!(pChild->GetStates() & FWL_WGTSTATE_Invisible))
      rtChilds.Union(rect);

    for (int32_t i = 0; i < kNeedRepaintHitPoints; i++) {
      if (hitPoint[i].bNotContainByDirty || hitPoint[i].bNotNeedRepaint)
        continue;
      if (!rtDirty.Contains(hitPoint[i].hitPoint)) {
        hitPoint[i].bNotContainByDirty = true;
        continue;
      }
      if (r.Contains(hitPoint[i].hitPoint))
        hitPoint[i].bNotNeedRepaint = true;
    }
    pChild =
        pChild->GetOwnerApp()->GetWidgetMgr()->GetNextSiblingWidget(pChild);
  } while (pChild);

  if (!bChildIntersectWithDirty)
    return true;
  if (bOrginPtIntersectWidthDirty && !bOrginPtIntersectWidthChild)
    return true;
  if (rtChilds.IsEmpty())
    return true;

  int32_t repaintPoint = kNeedRepaintHitPoints;
  for (int32_t i = 0; i < kNeedRepaintHitPoints; i++) {
    if (hitPoint[i].bNotNeedRepaint)
      repaintPoint--;
  }
  if (repaintPoint > 0)
    return true;

  pMatrix->TransformRect(rtChilds);
  if (rtChilds.Contains(rtDirty) || rtChilds.Contains(rtWidget))
    return false;
  return true;
}

CFWL_WidgetMgr::Item::Item() : CFWL_WidgetMgr::Item(nullptr) {}

CFWL_WidgetMgr::Item::Item(CFWL_Widget* widget)
    : pParent(nullptr),
      pOwner(nullptr),
      pChild(nullptr),
      pPrevious(nullptr),
      pNext(nullptr),
      pWidget(widget),
      iRedrawCounter(0)
#if (_FX_OS_ == _FX_WIN32_DESKTOP_) || (_FX_OS_ == _FX_WIN64_)
      ,
      bOutsideChanged(false)
#endif
{
}

CFWL_WidgetMgr::Item::~Item() {}
