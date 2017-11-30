// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_textlayout.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/css/cfx_csscomputedstyle.h"
#include "core/fxcrt/css/cfx_cssstyleselector.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_pathdata.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/cfde_textout.h"
#include "xfa/fxfa/cxfa_linkuserdata.h"
#include "xfa/fxfa/cxfa_loadercontext.h"
#include "xfa/fxfa/cxfa_pieceline.h"
#include "xfa/fxfa/cxfa_textparsecontext.h"
#include "xfa/fxfa/cxfa_textpiece.h"
#include "xfa/fxfa/cxfa_textprovider.h"
#include "xfa/fxfa/cxfa_texttabstopscontext.h"
#include "xfa/fxfa/cxfa_textuserdata.h"
#include "xfa/fxfa/parser/cxfa_fontdata.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_paradata.h"

#define XFA_LOADERCNTXTFLG_FILTERSPACE 0x001

CXFA_TextLayout::CXFA_TextLayout(CXFA_TextProvider* pTextProvider)
    : m_bHasBlock(false),
      m_pTextProvider(pTextProvider),
      m_pTextDataNode(nullptr),
      m_bRichText(false),
      m_iLines(0),
      m_fMaxWidth(0),
      m_bBlockContinue(true) {
  ASSERT(m_pTextProvider);
}

CXFA_TextLayout::~CXFA_TextLayout() {
  m_textParser.Reset();
  Unload();
}

void CXFA_TextLayout::Unload() {
  m_pieceLines.clear();
  m_pBreak.reset();
}

void CXFA_TextLayout::GetTextDataNode() {
  if (!m_pTextProvider)
    return;

  CXFA_Node* pNode = m_pTextProvider->GetTextNode(m_bRichText);
  if (pNode && m_bRichText)
    m_textParser.Reset();

  m_pTextDataNode = pNode;
}

CFX_XMLNode* CXFA_TextLayout::GetXMLContainerNode() {
  if (!m_bRichText)
    return nullptr;

  CFX_XMLNode* pXMLRoot = m_pTextDataNode->GetXMLMappingNode();
  if (!pXMLRoot)
    return nullptr;

  CFX_XMLNode* pXMLContainer = nullptr;
  for (CFX_XMLNode* pXMLChild = pXMLRoot->GetNodeItem(CFX_XMLNode::FirstChild);
       pXMLChild;
       pXMLChild = pXMLChild->GetNodeItem(CFX_XMLNode::NextSibling)) {
    if (pXMLChild->GetType() == FX_XMLNODE_Element) {
      CFX_XMLElement* pXMLElement = static_cast<CFX_XMLElement*>(pXMLChild);
      WideString wsTag = pXMLElement->GetLocalTagName();
      if (wsTag == L"body" || wsTag == L"html") {
        pXMLContainer = pXMLChild;
        break;
      }
    }
  }
  return pXMLContainer;
}

std::unique_ptr<CFX_RTFBreak> CXFA_TextLayout::CreateBreak(bool bDefault) {
  uint32_t dwStyle = FX_LAYOUTSTYLE_ExpandTab;
  if (!bDefault)
    dwStyle |= FX_LAYOUTSTYLE_Pagination;

  auto pBreak = pdfium::MakeUnique<CFX_RTFBreak>(dwStyle);
  pBreak->SetLineBreakTolerance(1);
  pBreak->SetFont(m_textParser.GetFont(m_pTextProvider, nullptr));
  pBreak->SetFontSize(m_textParser.GetFontSize(m_pTextProvider, nullptr));
  return pBreak;
}

void CXFA_TextLayout::InitBreak(float fLineWidth) {
  CXFA_ParaData paraData = m_pTextProvider->GetParaData();
  float fStart = 0;
  float fStartPos = 0;
  if (paraData.HasValidNode()) {
    CFX_RTFLineAlignment iAlign = CFX_RTFLineAlignment::Left;
    switch (paraData.GetHorizontalAlign()) {
      case XFA_AttributeEnum::Center:
        iAlign = CFX_RTFLineAlignment::Center;
        break;
      case XFA_AttributeEnum::Right:
        iAlign = CFX_RTFLineAlignment::Right;
        break;
      case XFA_AttributeEnum::Justify:
        iAlign = CFX_RTFLineAlignment::Justified;
        break;
      case XFA_AttributeEnum::JustifyAll:
        iAlign = CFX_RTFLineAlignment::Distributed;
        break;
      case XFA_AttributeEnum::Left:
      case XFA_AttributeEnum::Radix:
        break;
      default:
        NOTREACHED();
        break;
    }
    m_pBreak->SetAlignment(iAlign);

    fStart = paraData.GetMarginLeft();
    if (m_pTextProvider->IsCheckButtonAndAutoWidth()) {
      if (iAlign != CFX_RTFLineAlignment::Left)
        fLineWidth -= paraData.GetMarginRight();
    } else {
      fLineWidth -= paraData.GetMarginRight();
    }
    if (fLineWidth < 0)
      fLineWidth = fStart;

    fStartPos = fStart;
    float fIndent = paraData.GetTextIndent();
    if (fIndent > 0)
      fStartPos += fIndent;
  }

  m_pBreak->SetLineBoundary(fStart, fLineWidth);
  m_pBreak->SetLineStartPos(fStartPos);

  CXFA_FontData fontData = m_pTextProvider->GetFontData();
  if (fontData.HasValidNode()) {
    m_pBreak->SetHorizontalScale(
        static_cast<int32_t>(fontData.GetHorizontalScale()));
    m_pBreak->SetVerticalScale(
        static_cast<int32_t>(fontData.GetVerticalScale()));
    m_pBreak->SetCharSpace(fontData.GetLetterSpacing());
  }

  float fFontSize = m_textParser.GetFontSize(m_pTextProvider, nullptr);
  m_pBreak->SetFontSize(fFontSize);
  m_pBreak->SetFont(m_textParser.GetFont(m_pTextProvider, nullptr));
  m_pBreak->SetLineBreakTolerance(fFontSize * 0.2f);
}

void CXFA_TextLayout::InitBreak(CFX_CSSComputedStyle* pStyle,
                                CFX_CSSDisplay eDisplay,
                                float fLineWidth,
                                CFX_XMLNode* pXMLNode,
                                CFX_CSSComputedStyle* pParentStyle) {
  if (!pStyle) {
    InitBreak(fLineWidth);
    return;
  }

  if (eDisplay == CFX_CSSDisplay::Block ||
      eDisplay == CFX_CSSDisplay::ListItem) {
    CFX_RTFLineAlignment iAlign = CFX_RTFLineAlignment::Left;
    switch (pStyle->GetTextAlign()) {
      case CFX_CSSTextAlign::Right:
        iAlign = CFX_RTFLineAlignment::Right;
        break;
      case CFX_CSSTextAlign::Center:
        iAlign = CFX_RTFLineAlignment::Center;
        break;
      case CFX_CSSTextAlign::Justify:
        iAlign = CFX_RTFLineAlignment::Justified;
        break;
      case CFX_CSSTextAlign::JustifyAll:
        iAlign = CFX_RTFLineAlignment::Distributed;
        break;
      default:
        break;
    }
    m_pBreak->SetAlignment(iAlign);

    float fStart = 0;
    const CFX_CSSRect* pRect = pStyle->GetMarginWidth();
    const CFX_CSSRect* pPaddingRect = pStyle->GetPaddingWidth();
    if (pRect) {
      fStart = pRect->left.GetValue();
      fLineWidth -= pRect->right.GetValue();
      if (pPaddingRect) {
        fStart += pPaddingRect->left.GetValue();
        fLineWidth -= pPaddingRect->right.GetValue();
      }
      if (eDisplay == CFX_CSSDisplay::ListItem) {
        const CFX_CSSRect* pParRect = pParentStyle->GetMarginWidth();
        const CFX_CSSRect* pParPaddingRect = pParentStyle->GetPaddingWidth();
        if (pParRect) {
          fStart += pParRect->left.GetValue();
          fLineWidth -= pParRect->right.GetValue();
          if (pParPaddingRect) {
            fStart += pParPaddingRect->left.GetValue();
            fLineWidth -= pParPaddingRect->right.GetValue();
          }
        }
        CFX_CSSRect pNewRect;
        pNewRect.left.Set(CFX_CSSLengthUnit::Point, fStart);
        pNewRect.right.Set(CFX_CSSLengthUnit::Point, pRect->right.GetValue());
        pNewRect.top.Set(CFX_CSSLengthUnit::Point, pRect->top.GetValue());
        pNewRect.bottom.Set(CFX_CSSLengthUnit::Point, pRect->bottom.GetValue());
        pStyle->SetMarginWidth(pNewRect);
      }
    }
    m_pBreak->SetLineBoundary(fStart, fLineWidth);
    float fIndent = pStyle->GetTextIndent().GetValue();
    if (fIndent > 0)
      fStart += fIndent;

    m_pBreak->SetLineStartPos(fStart);
    m_pBreak->SetTabWidth(m_textParser.GetTabInterval(pStyle));
    if (!m_pTabstopContext)
      m_pTabstopContext = pdfium::MakeUnique<CXFA_TextTabstopsContext>();
    m_textParser.GetTabstops(pStyle, m_pTabstopContext.get());
    for (const auto& stop : m_pTabstopContext->m_tabstops)
      m_pBreak->AddPositionedTab(stop.fTabstops);
  }
  float fFontSize = m_textParser.GetFontSize(m_pTextProvider, pStyle);
  m_pBreak->SetFontSize(fFontSize);
  m_pBreak->SetLineBreakTolerance(fFontSize * 0.2f);
  m_pBreak->SetFont(m_textParser.GetFont(m_pTextProvider, pStyle));
  m_pBreak->SetHorizontalScale(
      m_textParser.GetHorScale(m_pTextProvider, pStyle, pXMLNode));
  m_pBreak->SetVerticalScale(m_textParser.GetVerScale(m_pTextProvider, pStyle));
  m_pBreak->SetCharSpace(pStyle->GetLetterSpacing().GetValue());
}

float CXFA_TextLayout::GetLayoutHeight() {
  if (!m_pLoader)
    return 0;

  if (m_pLoader->m_lineHeights.empty() && m_pLoader->m_fWidth > 0) {
    CFX_SizeF szMax(m_pLoader->m_fWidth, m_pLoader->m_fHeight);
    m_pLoader->m_bSaveLineHeight = true;
    m_pLoader->m_fLastPos = 0;
    CFX_SizeF szDef = CalcSize(szMax, szMax);
    m_pLoader->m_bSaveLineHeight = false;
    return szDef.height;
  }

  float fHeight = m_pLoader->m_fHeight;
  if (fHeight < 0.1f) {
    fHeight = 0;
    for (float value : m_pLoader->m_lineHeights)
      fHeight += value;
  }
  return fHeight;
}

float CXFA_TextLayout::StartLayout(float fWidth) {
  if (!m_pLoader)
    m_pLoader = pdfium::MakeUnique<CXFA_LoaderContext>();

  if (fWidth < 0 ||
      (m_pLoader->m_fWidth > -1 && fabs(fWidth - m_pLoader->m_fWidth) > 0)) {
    m_pLoader->m_lineHeights.clear();
    m_Blocks.clear();
    Unload();
    m_pLoader->m_fStartLineOffset = 0;
  }
  m_pLoader->m_fWidth = fWidth;

  if (fWidth >= 0)
    return fWidth;

  CFX_SizeF szMax;

  m_pLoader->m_bSaveLineHeight = true;
  m_pLoader->m_fLastPos = 0;
  CFX_SizeF szDef = CalcSize(szMax, szMax);
  m_pLoader->m_bSaveLineHeight = false;
  return szDef.width;
}

float CXFA_TextLayout::DoLayout(int32_t iBlockIndex,
                                float fCalcHeight,
                                float fContentAreaHeight,
                                float fTextHeight) {
  if (!m_pLoader)
    return fCalcHeight;

  int32_t iBlockCount = pdfium::CollectionSize<int32_t>(m_Blocks);
  float fHeight = fTextHeight;
  if (fHeight < 0)
    fHeight = GetLayoutHeight();

  m_pLoader->m_fHeight = fHeight;
  if (fContentAreaHeight < 0)
    return fCalcHeight;

  m_bHasBlock = true;
  if (iBlockCount == 0 && fHeight > 0) {
    fHeight = fTextHeight - GetLayoutHeight();
    if (fHeight > 0) {
      XFA_AttributeEnum iAlign = m_textParser.GetVAlign(m_pTextProvider);
      if (iAlign == XFA_AttributeEnum::Middle)
        fHeight /= 2.0f;
      else if (iAlign != XFA_AttributeEnum::Bottom)
        fHeight = 0;
      m_pLoader->m_fStartLineOffset = fHeight;
    }
  }

  float fLinePos = m_pLoader->m_fStartLineOffset;
  int32_t iLineIndex = 0;
  if (iBlockCount > 1) {
    if (iBlockCount >= (iBlockIndex + 1) * 2) {
      iLineIndex = m_Blocks[iBlockIndex * 2];
    } else {
      iLineIndex = m_Blocks[iBlockCount - 1] + m_Blocks[iBlockCount - 2];
    }
    if (!m_pLoader->m_BlocksHeight.empty()) {
      for (int32_t i = 0; i < iBlockIndex; i++)
        fLinePos -= m_pLoader->m_BlocksHeight[i * 2 + 1];
    }
  }

  int32_t iCount = pdfium::CollectionSize<int32_t>(m_pLoader->m_lineHeights);
  int32_t i = 0;
  for (i = iLineIndex; i < iCount; i++) {
    float fLineHeight = m_pLoader->m_lineHeights[i];
    if (i == iLineIndex && fLineHeight - fContentAreaHeight > 0.001)
      return 0;

    if (fLinePos + fLineHeight - fContentAreaHeight > 0.001) {
      if (iBlockCount >= (iBlockIndex + 1) * 2) {
        m_Blocks[iBlockIndex * 2] = iLineIndex;
        m_Blocks[iBlockIndex * 2 + 1] = i - iLineIndex;
      } else {
        m_Blocks.push_back(iLineIndex);
        m_Blocks.push_back(i - iLineIndex);
      }
      if (i == iLineIndex) {
        if (fCalcHeight <= fLinePos) {
          if (pdfium::CollectionSize<int32_t>(m_pLoader->m_BlocksHeight) >
                  iBlockIndex * 2 &&
              (m_pLoader->m_BlocksHeight[iBlockIndex * 2] == iBlockIndex)) {
            m_pLoader->m_BlocksHeight[iBlockIndex * 2 + 1] = fCalcHeight;
          } else {
            m_pLoader->m_BlocksHeight.push_back((float)iBlockIndex);
            m_pLoader->m_BlocksHeight.push_back(fCalcHeight);
          }
        }
        return fCalcHeight;
      }
      return fLinePos;
    }
    fLinePos += fLineHeight;
  }
  return fCalcHeight;
}

int32_t CXFA_TextLayout::CountBlocks() const {
  int32_t iCount = pdfium::CollectionSize<int32_t>(m_Blocks) / 2;
  return iCount > 0 ? iCount : 1;
}

CFX_SizeF CXFA_TextLayout::CalcSize(const CFX_SizeF& minSize,
                                    const CFX_SizeF& maxSize) {
  float width = maxSize.width;
  if (width < 1)
    width = 0xFFFF;

  m_pBreak = CreateBreak(false);
  float fLinePos = 0;
  m_iLines = 0;
  m_fMaxWidth = 0;
  Loader(width, fLinePos, false);
  if (fLinePos < 0.1f)
    fLinePos = m_textParser.GetFontSize(m_pTextProvider, nullptr);

  m_pTabstopContext.reset();
  return CFX_SizeF(m_fMaxWidth, fLinePos);
}

float CXFA_TextLayout::Layout(const CFX_SizeF& size) {
  if (size.width < 1)
    return 0.f;

  Unload();
  m_pBreak = CreateBreak(true);
  if (m_pLoader) {
    m_pLoader->m_iTotalLines = -1;
    m_pLoader->m_iChar = 0;
  }

  m_iLines = 0;
  float fLinePos = 0;
  Loader(size.width, fLinePos, true);
  UpdateAlign(size.height, fLinePos);
  m_pTabstopContext.reset();
  return fLinePos;
}

bool CXFA_TextLayout::Layout(int32_t iBlock) {
  if (!m_pLoader || iBlock < 0 || iBlock >= CountBlocks())
    return false;
  if (m_pLoader->m_fWidth < 1)
    return false;

  m_pLoader->m_iTotalLines = -1;
  m_iLines = 0;
  float fLinePos = 0;
  CXFA_Node* pNode = nullptr;
  CFX_SizeF szText(m_pLoader->m_fWidth, m_pLoader->m_fHeight);
  int32_t iCount = pdfium::CollectionSize<int32_t>(m_Blocks);
  int32_t iBlocksHeightCount =
      pdfium::CollectionSize<int32_t>(m_pLoader->m_BlocksHeight);
  iBlocksHeightCount /= 2;
  if (iBlock < iBlocksHeightCount)
    return true;
  if (iBlock == iBlocksHeightCount) {
    Unload();
    m_pBreak = CreateBreak(true);
    fLinePos = m_pLoader->m_fStartLineOffset;
    for (int32_t i = 0; i < iBlocksHeightCount; i++)
      fLinePos -= m_pLoader->m_BlocksHeight[i * 2 + 1];

    m_pLoader->m_iChar = 0;
    if (iCount > 1)
      m_pLoader->m_iTotalLines = m_Blocks[iBlock * 2 + 1];

    Loader(szText.width, fLinePos, true);
    if (iCount == 0 && m_pLoader->m_fStartLineOffset < 0.1f)
      UpdateAlign(szText.height, fLinePos);
  } else if (m_pTextDataNode) {
    iBlock *= 2;
    if (iBlock < iCount - 2)
      m_pLoader->m_iTotalLines = m_Blocks[iBlock + 1];

    m_pBreak->Reset();
    if (m_bRichText) {
      CFX_XMLNode* pContainerNode = GetXMLContainerNode();
      if (!pContainerNode)
        return true;

      CFX_XMLNode* pXMLNode = m_pLoader->m_pXMLNode;
      if (!pXMLNode)
        return true;

      CFX_XMLNode* pSaveXMLNode = m_pLoader->m_pXMLNode;
      for (; pXMLNode;
           pXMLNode = pXMLNode->GetNodeItem(CFX_XMLNode::NextSibling)) {
        if (!LoadRichText(pXMLNode, szText.width, fLinePos,
                          m_pLoader->m_pParentStyle, true, nullptr)) {
          break;
        }
      }
      while (!pXMLNode) {
        pXMLNode = pSaveXMLNode->GetNodeItem(CFX_XMLNode::Parent);
        if (pXMLNode == pContainerNode)
          break;
        if (!LoadRichText(pXMLNode, szText.width, fLinePos,
                          m_pLoader->m_pParentStyle, true, nullptr, false)) {
          break;
        }
        pSaveXMLNode = pXMLNode;
        pXMLNode = pXMLNode->GetNodeItem(CFX_XMLNode::NextSibling);
        if (!pXMLNode)
          continue;
        for (; pXMLNode;
             pXMLNode = pXMLNode->GetNodeItem(CFX_XMLNode::NextSibling)) {
          if (!LoadRichText(pXMLNode, szText.width, fLinePos,
                            m_pLoader->m_pParentStyle, true, nullptr)) {
            break;
          }
        }
      }
    } else {
      pNode = m_pLoader->m_pNode;
      if (!pNode)
        return true;
      LoadText(pNode, szText.width, fLinePos, true);
    }
  }
  if (iBlock == iCount) {
    m_pTabstopContext.reset();
    m_pLoader.reset();
  }
  return true;
}

void CXFA_TextLayout::ItemBlocks(const CFX_RectF& rtText, int32_t iBlockIndex) {
  if (!m_pLoader)
    return;

  int32_t iCountHeight =
      pdfium::CollectionSize<int32_t>(m_pLoader->m_lineHeights);
  if (iCountHeight == 0)
    return;

  bool bEndItem = true;
  int32_t iBlockCount = pdfium::CollectionSize<int32_t>(m_Blocks);
  float fLinePos = m_pLoader->m_fStartLineOffset;
  int32_t iLineIndex = 0;
  if (iBlockIndex > 0) {
    int32_t iBlockHeightCount =
        pdfium::CollectionSize<int32_t>(m_pLoader->m_BlocksHeight);
    iBlockHeightCount /= 2;
    if (iBlockHeightCount >= iBlockIndex) {
      for (int32_t i = 0; i < iBlockIndex; i++)
        fLinePos -= m_pLoader->m_BlocksHeight[i * 2 + 1];
    } else {
      fLinePos = 0;
    }
    iLineIndex = m_Blocks[iBlockCount - 1] + m_Blocks[iBlockCount - 2];
  }

  int32_t i = 0;
  for (i = iLineIndex; i < iCountHeight; i++) {
    float fLineHeight = m_pLoader->m_lineHeights[i];
    if (fLinePos + fLineHeight - rtText.height > 0.001) {
      m_Blocks.push_back(iLineIndex);
      m_Blocks.push_back(i - iLineIndex);
      bEndItem = false;
      break;
    }
    fLinePos += fLineHeight;
  }
  if (iCountHeight > 0 && (i - iLineIndex) > 0 && bEndItem) {
    m_Blocks.push_back(iLineIndex);
    m_Blocks.push_back(i - iLineIndex);
  }
}

bool CXFA_TextLayout::DrawString(CFX_RenderDevice* pFxDevice,
                                 const CFX_Matrix& tmDoc2Device,
                                 const CFX_RectF& rtClip,
                                 int32_t iBlock) {
  if (!pFxDevice)
    return false;

  pFxDevice->SaveState();
  pFxDevice->SetClip_Rect(rtClip);

  if (m_pieceLines.empty()) {
    int32_t iBlockCount = CountBlocks();
    for (int32_t i = 0; i < iBlockCount; i++)
      Layout(i);
  }

  FXTEXT_CHARPOS* pCharPos = nullptr;
  int32_t iCharCount = 0;
  int32_t iLineStart = 0;
  int32_t iPieceLines = pdfium::CollectionSize<int32_t>(m_pieceLines);
  int32_t iCount = pdfium::CollectionSize<int32_t>(m_Blocks);
  if (iCount > 0) {
    iBlock *= 2;
    if (iBlock < iCount) {
      iLineStart = m_Blocks[iBlock];
      iPieceLines = m_Blocks[iBlock + 1];
    } else {
      iPieceLines = 0;
    }
  }

  for (int32_t i = 0; i < iPieceLines; i++) {
    if (i + iLineStart >= pdfium::CollectionSize<int32_t>(m_pieceLines))
      break;

    CXFA_PieceLine* pPieceLine = m_pieceLines[i + iLineStart].get();
    int32_t iPieces = pdfium::CollectionSize<int32_t>(pPieceLine->m_textPieces);
    int32_t j = 0;
    for (j = 0; j < iPieces; j++) {
      const CXFA_TextPiece* pPiece = pPieceLine->m_textPieces[j].get();
      int32_t iChars = pPiece->iChars;
      if (iCharCount < iChars) {
        FX_Free(pCharPos);
        pCharPos = FX_Alloc(FXTEXT_CHARPOS, iChars);
        iCharCount = iChars;
      }
      memset(pCharPos, 0, iCharCount * sizeof(FXTEXT_CHARPOS));
      RenderString(pFxDevice, pPieceLine, j, pCharPos, tmDoc2Device);
    }
    for (j = 0; j < iPieces; j++)
      RenderPath(pFxDevice, pPieceLine, j, pCharPos, tmDoc2Device);
  }
  pFxDevice->RestoreState(false);
  FX_Free(pCharPos);
  return iPieceLines > 0;
}

void CXFA_TextLayout::UpdateAlign(float fHeight, float fBottom) {
  fHeight -= fBottom;
  if (fHeight < 0.1f)
    return;

  switch (m_textParser.GetVAlign(m_pTextProvider)) {
    case XFA_AttributeEnum::Middle:
      fHeight /= 2.0f;
      break;
    case XFA_AttributeEnum::Bottom:
      break;
    default:
      return;
  }

  for (const auto& pPieceLine : m_pieceLines) {
    for (const auto& pPiece : pPieceLine->m_textPieces)
      pPiece->rtPiece.top += fHeight;
  }
}

bool CXFA_TextLayout::Loader(float textWidth,
                             float& fLinePos,
                             bool bSavePieces) {
  GetTextDataNode();
  if (!m_pTextDataNode)
    return true;

  if (m_bRichText) {
    CFX_XMLNode* pXMLContainer = GetXMLContainerNode();
    if (pXMLContainer) {
      if (!m_textParser.IsParsed())
        m_textParser.DoParse(pXMLContainer, m_pTextProvider);

      auto pRootStyle = m_textParser.CreateRootStyle(m_pTextProvider);
      LoadRichText(pXMLContainer, textWidth, fLinePos, pRootStyle, bSavePieces,
                   nullptr);
    }
  } else {
    LoadText(m_pTextDataNode, textWidth, fLinePos, bSavePieces);
  }
  return true;
}

void CXFA_TextLayout::LoadText(CXFA_Node* pNode,
                               float textWidth,
                               float& fLinePos,
                               bool bSavePieces) {
  InitBreak(textWidth);

  CXFA_ParaData paraData = m_pTextProvider->GetParaData();
  float fSpaceAbove = 0;
  if (paraData.HasValidNode()) {
    fSpaceAbove = paraData.GetSpaceAbove();
    if (fSpaceAbove < 0.1f)
      fSpaceAbove = 0;

    switch (paraData.GetVerticalAlign()) {
      case XFA_AttributeEnum::Top:
      case XFA_AttributeEnum::Middle:
      case XFA_AttributeEnum::Bottom: {
        fLinePos += fSpaceAbove;
        break;
      }
      default:
        NOTREACHED();
        break;
    }
  }

  WideString wsText = pNode->JSNode()->GetContent(false);
  wsText.TrimRight(L" ");
  bool bRet = AppendChar(wsText, fLinePos, fSpaceAbove, bSavePieces);
  if (bRet && m_pLoader)
    m_pLoader->m_pNode = pNode;
  else
    EndBreak(CFX_BreakType::Paragraph, fLinePos, bSavePieces);
}

bool CXFA_TextLayout::LoadRichText(
    CFX_XMLNode* pXMLNode,
    float textWidth,
    float& fLinePos,
    const RetainPtr<CFX_CSSComputedStyle>& pParentStyle,
    bool bSavePieces,
    RetainPtr<CXFA_LinkUserData> pLinkData,
    bool bEndBreak,
    bool bIsOl,
    int32_t iLiCount) {
  if (!pXMLNode)
    return false;

  CXFA_TextParseContext* pContext =
      m_textParser.GetParseContextFromMap(pXMLNode);
  CFX_CSSDisplay eDisplay = CFX_CSSDisplay::None;
  bool bContentNode = false;
  float fSpaceBelow = 0;
  RetainPtr<CFX_CSSComputedStyle> pStyle;
  WideString wsName;
  if (bEndBreak) {
    bool bCurOl = false;
    bool bCurLi = false;
    CFX_XMLElement* pElement = nullptr;
    if (pContext) {
      if (m_bBlockContinue ||
          (m_pLoader && pXMLNode == m_pLoader->m_pXMLNode)) {
        m_bBlockContinue = true;
      }
      if (pXMLNode->GetType() == FX_XMLNODE_Text) {
        bContentNode = true;
      } else if (pXMLNode->GetType() == FX_XMLNODE_Element) {
        pElement = static_cast<CFX_XMLElement*>(pXMLNode);
        wsName = pElement->GetLocalTagName();
      }
      if (wsName == L"ol") {
        bIsOl = true;
        bCurOl = true;
      }
      if (m_bBlockContinue || bContentNode == false) {
        eDisplay = pContext->GetDisplay();
        if (eDisplay != CFX_CSSDisplay::Block &&
            eDisplay != CFX_CSSDisplay::Inline &&
            eDisplay != CFX_CSSDisplay::ListItem) {
          return true;
        }

        pStyle = m_textParser.ComputeStyle(pXMLNode, pParentStyle.Get());
        InitBreak(bContentNode ? pParentStyle.Get() : pStyle.Get(), eDisplay,
                  textWidth, pXMLNode, pParentStyle.Get());
        if ((eDisplay == CFX_CSSDisplay::Block ||
             eDisplay == CFX_CSSDisplay::ListItem) &&
            pStyle &&
            (wsName.IsEmpty() || (wsName != L"body" && wsName != L"html" &&
                                  wsName != L"ol" && wsName != L"ul"))) {
          const CFX_CSSRect* pRect = pStyle->GetMarginWidth();
          if (pRect) {
            fLinePos += pRect->top.GetValue();
            fSpaceBelow = pRect->bottom.GetValue();
          }
        }

        if (wsName == L"a") {
          ASSERT(pElement);
          WideString wsLinkContent = pElement->GetString(L"href");
          if (!wsLinkContent.IsEmpty()) {
            pLinkData = pdfium::MakeRetain<CXFA_LinkUserData>(
                wsLinkContent.GetBuffer(wsLinkContent.GetLength()));
            wsLinkContent.ReleaseBuffer(wsLinkContent.GetLength());
          }
        }

        int32_t iTabCount = m_textParser.CountTabs(
            bContentNode ? pParentStyle.Get() : pStyle.Get());
        bool bSpaceRun = m_textParser.IsSpaceRun(
            bContentNode ? pParentStyle.Get() : pStyle.Get());
        WideString wsText;
        if (bContentNode && iTabCount == 0) {
          wsText = static_cast<CFX_XMLText*>(pXMLNode)->GetText();
        } else if (wsName == L"br") {
          wsText = L'\n';
        } else if (wsName == L"li") {
          bCurLi = true;
          if (bIsOl)
            wsText = WideString::Format(L"%d.  ", iLiCount);
          else
            wsText = 0x00B7 + WideStringView(L"  ", 1);
        } else if (!bContentNode) {
          if (iTabCount > 0) {
            while (iTabCount-- > 0)
              wsText += L'\t';
          } else {
            m_textParser.GetEmbbedObj(m_pTextProvider, pXMLNode, wsText);
          }
        }

        int32_t iLength = wsText.GetLength();
        if (iLength > 0 && bContentNode && !bSpaceRun)
          ProcessText(wsText);

        if (m_pLoader) {
          if (wsText.GetLength() > 0 &&
              (m_pLoader->m_dwFlags & XFA_LOADERCNTXTFLG_FILTERSPACE)) {
            wsText.TrimLeft(L" ");
          }
          if (CFX_CSSDisplay::Block == eDisplay) {
            m_pLoader->m_dwFlags |= XFA_LOADERCNTXTFLG_FILTERSPACE;
          } else if (CFX_CSSDisplay::Inline == eDisplay &&
                     (m_pLoader->m_dwFlags & XFA_LOADERCNTXTFLG_FILTERSPACE)) {
            m_pLoader->m_dwFlags &= ~XFA_LOADERCNTXTFLG_FILTERSPACE;
          } else if (wsText.GetLength() > 0 &&
                     (0x20 == wsText[wsText.GetLength() - 1])) {
            m_pLoader->m_dwFlags |= XFA_LOADERCNTXTFLG_FILTERSPACE;
          } else if (wsText.GetLength() != 0) {
            m_pLoader->m_dwFlags &= ~XFA_LOADERCNTXTFLG_FILTERSPACE;
          }
        }

        if (wsText.GetLength() > 0) {
          if (!m_pLoader || m_pLoader->m_iChar == 0) {
            auto pUserData = pdfium::MakeRetain<CXFA_TextUserData>(
                bContentNode ? pParentStyle : pStyle, pLinkData);
            m_pBreak->SetUserData(pUserData);
          }

          if (AppendChar(wsText, fLinePos, 0, bSavePieces)) {
            if (m_pLoader)
              m_pLoader->m_dwFlags &= ~XFA_LOADERCNTXTFLG_FILTERSPACE;
            if (IsEnd(bSavePieces)) {
              if (m_pLoader && m_pLoader->m_iTotalLines > -1) {
                m_pLoader->m_pXMLNode = pXMLNode;
                m_pLoader->m_pParentStyle = pParentStyle;
              }
              return false;
            }
            return true;
          }
        }
      }
    }

    for (CFX_XMLNode* pChildNode =
             pXMLNode->GetNodeItem(CFX_XMLNode::FirstChild);
         pChildNode;
         pChildNode = pChildNode->GetNodeItem(CFX_XMLNode::NextSibling)) {
      if (bCurOl)
        iLiCount++;

      if (!LoadRichText(pChildNode, textWidth, fLinePos,
                        pContext ? pStyle : pParentStyle, bSavePieces,
                        pLinkData, true, bIsOl, iLiCount))
        return false;
    }

    if (m_pLoader) {
      if (CFX_CSSDisplay::Block == eDisplay)
        m_pLoader->m_dwFlags |= XFA_LOADERCNTXTFLG_FILTERSPACE;
    }
    if (bCurLi)
      EndBreak(CFX_BreakType::Line, fLinePos, bSavePieces);
  } else {
    if (pContext)
      eDisplay = pContext->GetDisplay();
  }

  if (m_bBlockContinue) {
    if (pContext && !bContentNode) {
      CFX_BreakType dwStatus = (eDisplay == CFX_CSSDisplay::Block)
                                   ? CFX_BreakType::Paragraph
                                   : CFX_BreakType::Piece;
      EndBreak(dwStatus, fLinePos, bSavePieces);
      if (eDisplay == CFX_CSSDisplay::Block) {
        fLinePos += fSpaceBelow;
        if (m_pTabstopContext)
          m_pTabstopContext->RemoveAll();
      }
      if (IsEnd(bSavePieces)) {
        if (m_pLoader && m_pLoader->m_iTotalLines > -1) {
          m_pLoader->m_pXMLNode =
              pXMLNode->GetNodeItem(CFX_XMLNode::NextSibling);
          m_pLoader->m_pParentStyle = pParentStyle;
        }
        return false;
      }
    }
  }
  return true;
}

bool CXFA_TextLayout::AppendChar(const WideString& wsText,
                                 float& fLinePos,
                                 float fSpaceAbove,
                                 bool bSavePieces) {
  CFX_BreakType dwStatus = CFX_BreakType::None;
  int32_t iChar = 0;
  if (m_pLoader)
    iChar = m_pLoader->m_iChar;

  int32_t iLength = wsText.GetLength();
  for (int32_t i = iChar; i < iLength; i++) {
    wchar_t wch = wsText[i];
    if (wch == 0xA0)
      wch = 0x20;

    dwStatus = m_pBreak->AppendChar(wch);
    if (dwStatus != CFX_BreakType::None && dwStatus != CFX_BreakType::Piece) {
      AppendTextLine(dwStatus, fLinePos, bSavePieces);
      if (IsEnd(bSavePieces)) {
        if (m_pLoader)
          m_pLoader->m_iChar = i;
        return true;
      }
      if (dwStatus == CFX_BreakType::Paragraph && m_bRichText)
        fLinePos += fSpaceAbove;
    }
  }
  if (m_pLoader)
    m_pLoader->m_iChar = 0;

  return false;
}

bool CXFA_TextLayout::IsEnd(bool bSavePieces) {
  if (!bSavePieces)
    return false;
  if (m_pLoader && m_pLoader->m_iTotalLines > 0)
    return m_iLines >= m_pLoader->m_iTotalLines;
  return false;
}

void CXFA_TextLayout::ProcessText(WideString& wsText) {
  int32_t iLen = wsText.GetLength();
  if (iLen == 0)
    return;

  wchar_t* psz = wsText.GetBuffer(iLen);
  int32_t iTrimLeft = 0;
  wchar_t wch = 0, wPrev = 0;
  for (int32_t i = 0; i < iLen; i++) {
    wch = psz[i];
    if (wch < 0x20)
      wch = 0x20;
    if (wch == 0x20 && wPrev == 0x20)
      continue;

    wPrev = wch;
    psz[iTrimLeft++] = wch;
  }
  wsText.ReleaseBuffer(iLen);
  wsText = wsText.Left(iTrimLeft);
}

void CXFA_TextLayout::EndBreak(CFX_BreakType dwStatus,
                               float& fLinePos,
                               bool bSavePieces) {
  dwStatus = m_pBreak->EndBreak(dwStatus);
  if (dwStatus != CFX_BreakType::None && dwStatus != CFX_BreakType::Piece)
    AppendTextLine(dwStatus, fLinePos, bSavePieces, true);
}

void CXFA_TextLayout::DoTabstops(CFX_CSSComputedStyle* pStyle,
                                 CXFA_PieceLine* pPieceLine) {
  if (!pStyle || !pPieceLine)
    return;

  if (!m_pTabstopContext || m_pTabstopContext->m_tabstops.empty())
    return;

  int32_t iPieces = pdfium::CollectionSize<int32_t>(pPieceLine->m_textPieces);
  if (iPieces == 0)
    return;

  CXFA_TextPiece* pPiece = pPieceLine->m_textPieces[iPieces - 1].get();
  int32_t& iTabstopsIndex = m_pTabstopContext->m_iTabIndex;
  int32_t iCount = m_textParser.CountTabs(pStyle);
  if (!pdfium::IndexInBounds(m_pTabstopContext->m_tabstops, iTabstopsIndex))
    return;

  if (iCount > 0) {
    iTabstopsIndex++;
    m_pTabstopContext->m_bTabstops = true;
    float fRight = 0;
    if (iPieces > 1) {
      CXFA_TextPiece* p = pPieceLine->m_textPieces[iPieces - 2].get();
      fRight = p->rtPiece.right();
    }
    m_pTabstopContext->m_fTabWidth =
        pPiece->rtPiece.width + pPiece->rtPiece.left - fRight;
  } else if (iTabstopsIndex > -1) {
    float fLeft = 0;
    if (m_pTabstopContext->m_bTabstops) {
      uint32_t dwAlign = m_pTabstopContext->m_tabstops[iTabstopsIndex].dwAlign;
      if (dwAlign == FX_HashCode_GetW(L"center", false)) {
        fLeft = pPiece->rtPiece.width / 2.0f;
      } else if (dwAlign == FX_HashCode_GetW(L"right", false) ||
                 dwAlign == FX_HashCode_GetW(L"before", false)) {
        fLeft = pPiece->rtPiece.width;
      } else if (dwAlign == FX_HashCode_GetW(L"decimal", false)) {
        int32_t iChars = pPiece->iChars;
        for (int32_t i = 0; i < iChars; i++) {
          if (pPiece->szText[i] == L'.')
            break;

          fLeft += pPiece->Widths[i] / 20000.0f;
        }
      }
      m_pTabstopContext->m_fLeft =
          std::min(fLeft, m_pTabstopContext->m_fTabWidth);
      m_pTabstopContext->m_bTabstops = false;
      m_pTabstopContext->m_fTabWidth = 0;
    }
    pPiece->rtPiece.left -= m_pTabstopContext->m_fLeft;
  }
}

void CXFA_TextLayout::AppendTextLine(CFX_BreakType dwStatus,
                                     float& fLinePos,
                                     bool bSavePieces,
                                     bool bEndBreak) {
  int32_t iPieces = m_pBreak->CountBreakPieces();
  if (iPieces < 1)
    return;

  RetainPtr<CFX_CSSComputedStyle> pStyle;
  if (bSavePieces) {
    auto pNew = pdfium::MakeUnique<CXFA_PieceLine>();
    CXFA_PieceLine* pPieceLine = pNew.get();
    m_pieceLines.push_back(std::move(pNew));
    if (m_pTabstopContext)
      m_pTabstopContext->Reset();

    float fLineStep = 0, fBaseLine = 0;
    int32_t i = 0;
    for (i = 0; i < iPieces; i++) {
      const CFX_BreakPiece* pPiece = m_pBreak->GetBreakPieceUnstable(i);
      CXFA_TextUserData* pUserData = pPiece->m_pUserData.Get();
      if (pUserData)
        pStyle = pUserData->m_pStyle;
      float fVerScale = pPiece->m_iVerticalScale / 100.0f;

      auto pTP = pdfium::MakeUnique<CXFA_TextPiece>();
      pTP->iChars = pPiece->m_iChars;
      pTP->szText = pPiece->GetString();
      pTP->Widths = pPiece->GetWidths();
      pTP->iBidiLevel = pPiece->m_iBidiLevel;
      pTP->iHorScale = pPiece->m_iHorizontalScale;
      pTP->iVerScale = pPiece->m_iVerticalScale;
      m_textParser.GetUnderline(m_pTextProvider, pStyle.Get(), pTP->iUnderline,
                                pTP->iPeriod);
      m_textParser.GetLinethrough(m_pTextProvider, pStyle.Get(),
                                  pTP->iLineThrough);
      pTP->dwColor = m_textParser.GetColor(m_pTextProvider, pStyle.Get());
      pTP->pFont = m_textParser.GetFont(m_pTextProvider, pStyle.Get());
      pTP->fFontSize = m_textParser.GetFontSize(m_pTextProvider, pStyle.Get());
      pTP->rtPiece.left = pPiece->m_iStartPos / 20000.0f;
      pTP->rtPiece.width = pPiece->m_iWidth / 20000.0f;
      pTP->rtPiece.height = (float)pPiece->m_iFontSize * fVerScale / 20.0f;
      float fBaseLineTemp =
          m_textParser.GetBaseline(m_pTextProvider, pStyle.Get());
      pTP->rtPiece.top = fBaseLineTemp;

      float fLineHeight = m_textParser.GetLineHeight(
          m_pTextProvider, pStyle.Get(), m_iLines == 0, fVerScale);
      if (fBaseLineTemp > 0) {
        float fLineHeightTmp = fBaseLineTemp + pTP->rtPiece.height;
        if (fLineHeight < fLineHeightTmp)
          fLineHeight = fLineHeightTmp;
        else
          fBaseLineTemp = 0;
      } else if (fBaseLine < -fBaseLineTemp) {
        fBaseLine = -fBaseLineTemp;
      }
      fLineStep = std::max(fLineStep, fLineHeight);
      pTP->pLinkData = pUserData ? pUserData->m_pLinkData : nullptr;
      pPieceLine->m_textPieces.push_back(std::move(pTP));
      DoTabstops(pStyle.Get(), pPieceLine);
    }
    for (const auto& pTP : pPieceLine->m_textPieces) {
      float& fTop = pTP->rtPiece.top;
      float fBaseLineTemp = fTop;
      fTop = fLinePos + fLineStep - pTP->rtPiece.height - fBaseLineTemp;
      fTop = std::max(0.0f, fTop);
    }
    fLinePos += fLineStep + fBaseLine;
  } else {
    float fLineStep = 0;
    float fLineWidth = 0;
    for (int32_t i = 0; i < iPieces; i++) {
      const CFX_BreakPiece* pPiece = m_pBreak->GetBreakPieceUnstable(i);
      CXFA_TextUserData* pUserData = pPiece->m_pUserData.Get();
      if (pUserData)
        pStyle = pUserData->m_pStyle;
      float fVerScale = pPiece->m_iVerticalScale / 100.0f;
      float fBaseLine = m_textParser.GetBaseline(m_pTextProvider, pStyle.Get());
      float fLineHeight = m_textParser.GetLineHeight(
          m_pTextProvider, pStyle.Get(), m_iLines == 0, fVerScale);
      if (fBaseLine > 0) {
        float fLineHeightTmp =
            fBaseLine + (float)pPiece->m_iFontSize * fVerScale / 20.0f;
        if (fLineHeight < fLineHeightTmp) {
          fLineHeight = fLineHeightTmp;
        }
      }
      fLineStep = std::max(fLineStep, fLineHeight);
      fLineWidth += pPiece->m_iWidth / 20000.0f;
    }
    fLinePos += fLineStep;
    m_fMaxWidth = std::max(m_fMaxWidth, fLineWidth);
    if (m_pLoader && m_pLoader->m_bSaveLineHeight) {
      float fHeight = fLinePos - m_pLoader->m_fLastPos;
      m_pLoader->m_fLastPos = fLinePos;
      m_pLoader->m_lineHeights.push_back(fHeight);
    }
  }

  m_pBreak->ClearBreakPieces();
  if (dwStatus == CFX_BreakType::Paragraph) {
    m_pBreak->Reset();
    if (!pStyle && bEndBreak) {
      CXFA_ParaData paraData = m_pTextProvider->GetParaData();
      if (paraData.HasValidNode()) {
        float fStartPos = paraData.GetMarginLeft();
        float fIndent = paraData.GetTextIndent();
        if (fIndent > 0)
          fStartPos += fIndent;

        float fSpaceBelow = paraData.GetSpaceBelow();
        if (fSpaceBelow < 0.1f)
          fSpaceBelow = 0;

        m_pBreak->SetLineStartPos(fStartPos);
        fLinePos += fSpaceBelow;
      }
    }
  }

  if (pStyle) {
    float fStart = 0;
    const CFX_CSSRect* pRect = pStyle->GetMarginWidth();
    if (pRect)
      fStart = pRect->left.GetValue();

    float fTextIndent = pStyle->GetTextIndent().GetValue();
    if (fTextIndent < 0)
      fStart -= fTextIndent;

    m_pBreak->SetLineStartPos(fStart);
  }
  m_iLines++;
}

void CXFA_TextLayout::RenderString(CFX_RenderDevice* pDevice,
                                   CXFA_PieceLine* pPieceLine,
                                   int32_t iPiece,
                                   FXTEXT_CHARPOS* pCharPos,
                                   const CFX_Matrix& tmDoc2Device) {
  const CXFA_TextPiece* pPiece = pPieceLine->m_textPieces[iPiece].get();
  int32_t iCount = GetDisplayPos(pPiece, pCharPos);
  if (iCount > 0) {
    CFDE_TextOut::DrawString(pDevice, pPiece->dwColor, pPiece->pFont, pCharPos,
                             iCount, pPiece->fFontSize, &tmDoc2Device);
  }
  pPieceLine->m_charCounts.push_back(iCount);
}

void CXFA_TextLayout::RenderPath(CFX_RenderDevice* pDevice,
                                 CXFA_PieceLine* pPieceLine,
                                 int32_t iPiece,
                                 FXTEXT_CHARPOS* pCharPos,
                                 const CFX_Matrix& tmDoc2Device) {
  CXFA_TextPiece* pPiece = pPieceLine->m_textPieces[iPiece].get();
  bool bNoUnderline = pPiece->iUnderline < 1 || pPiece->iUnderline > 2;
  bool bNoLineThrough = pPiece->iLineThrough < 1 || pPiece->iLineThrough > 2;
  if (bNoUnderline && bNoLineThrough)
    return;

  CFX_PathData path;
  int32_t iChars = GetDisplayPos(pPiece, pCharPos);
  if (iChars > 0) {
    CFX_PointF pt1, pt2;
    float fEndY = pCharPos[0].m_Origin.y + 1.05f;
    if (pPiece->iPeriod == XFA_AttributeEnum::Word) {
      for (int32_t i = 0; i < pPiece->iUnderline; i++) {
        for (int32_t j = 0; j < iChars; j++) {
          pt1.x = pCharPos[j].m_Origin.x;
          pt2.x =
              pt1.x + pCharPos[j].m_FontCharWidth * pPiece->fFontSize / 1000.0f;
          pt1.y = pt2.y = fEndY;
          path.AppendLine(pt1, pt2);
        }
        fEndY += 2.0f;
      }
    } else {
      pt1.x = pCharPos[0].m_Origin.x;
      pt2.x =
          pCharPos[iChars - 1].m_Origin.x +
          pCharPos[iChars - 1].m_FontCharWidth * pPiece->fFontSize / 1000.0f;
      for (int32_t i = 0; i < pPiece->iUnderline; i++) {
        pt1.y = pt2.y = fEndY;
        path.AppendLine(pt1, pt2);
        fEndY += 2.0f;
      }
    }
    fEndY = pCharPos[0].m_Origin.y - pPiece->rtPiece.height * 0.25f;
    pt1.x = pCharPos[0].m_Origin.x;
    pt2.x = pCharPos[iChars - 1].m_Origin.x +
            pCharPos[iChars - 1].m_FontCharWidth * pPiece->fFontSize / 1000.0f;
    for (int32_t i = 0; i < pPiece->iLineThrough; i++) {
      pt1.y = pt2.y = fEndY;
      path.AppendLine(pt1, pt2);
      fEndY += 2.0f;
    }
  } else {
    if (bNoLineThrough &&
        (bNoUnderline || pPiece->iPeriod != XFA_AttributeEnum::All)) {
      return;
    }
    int32_t iCharsTmp = 0;
    int32_t iPiecePrev = iPiece;
    int32_t iPieceNext = iPiece;
    while (iPiecePrev > 0) {
      iPiecePrev--;
      iCharsTmp = pPieceLine->m_charCounts[iPiecePrev];
      if (iCharsTmp > 0)
        break;
    }
    if (iCharsTmp == 0)
      return;

    iCharsTmp = 0;
    int32_t iPieces = pdfium::CollectionSize<int32_t>(pPieceLine->m_textPieces);
    while (iPieceNext < iPieces - 1) {
      iPieceNext++;
      iCharsTmp = pPieceLine->m_charCounts[iPieceNext];
      if (iCharsTmp > 0)
        break;
    }
    if (iCharsTmp == 0)
      return;

    float fOrgX = 0.0f;
    float fEndX = 0.0f;
    pPiece = pPieceLine->m_textPieces[iPiecePrev].get();
    iChars = GetDisplayPos(pPiece, pCharPos);
    if (iChars < 1)
      return;

    fOrgX = pCharPos[iChars - 1].m_Origin.x +
            pCharPos[iChars - 1].m_FontCharWidth * pPiece->fFontSize / 1000.0f;
    pPiece = pPieceLine->m_textPieces[iPieceNext].get();
    iChars = GetDisplayPos(pPiece, pCharPos);
    if (iChars < 1)
      return;

    fEndX = pCharPos[0].m_Origin.x;
    CFX_PointF pt1;
    CFX_PointF pt2;
    pt1.x = fOrgX;
    pt2.x = fEndX;
    float fEndY = pCharPos[0].m_Origin.y + 1.05f;
    for (int32_t i = 0; i < pPiece->iUnderline; i++) {
      pt1.y = fEndY;
      pt2.y = fEndY;
      path.AppendLine(pt1, pt2);
      fEndY += 2.0f;
    }
    fEndY = pCharPos[0].m_Origin.y - pPiece->rtPiece.height * 0.25f;
    for (int32_t i = 0; i < pPiece->iLineThrough; i++) {
      pt1.y = fEndY;
      pt2.y = fEndY;
      path.AppendLine(pt1, pt2);
      fEndY += 2.0f;
    }
  }

  CFX_GraphStateData graphState;
  graphState.m_LineCap = CFX_GraphStateData::LineCapButt;
  graphState.m_LineJoin = CFX_GraphStateData::LineJoinMiter;
  graphState.m_LineWidth = 1;
  graphState.m_MiterLimit = 10;
  graphState.m_DashPhase = 0;
  pDevice->DrawPath(&path, &tmDoc2Device, &graphState, 0, pPiece->dwColor, 0);
}

int32_t CXFA_TextLayout::GetDisplayPos(const CXFA_TextPiece* pPiece,
                                       FXTEXT_CHARPOS* pCharPos,
                                       bool bCharCode) {
  if (!pPiece)
    return 0;

  FX_RTFTEXTOBJ tr;
  if (!ToRun(pPiece, &tr))
    return 0;
  return m_pBreak->GetDisplayPos(&tr, pCharPos, bCharCode);
}

bool CXFA_TextLayout::ToRun(const CXFA_TextPiece* pPiece, FX_RTFTEXTOBJ* tr) {
  int32_t iLength = pPiece->iChars;
  if (iLength < 1)
    return false;

  tr->pStr = pPiece->szText;
  tr->pFont = pPiece->pFont;
  tr->pRect = &pPiece->rtPiece;
  tr->pWidths = pPiece->Widths;
  tr->iLength = iLength;
  tr->fFontSize = pPiece->fFontSize;
  tr->iBidiLevel = pPiece->iBidiLevel;
  tr->wLineBreakChar = L'\n';
  tr->iVerticalScale = pPiece->iVerScale;
  tr->iHorizontalScale = pPiece->iHorScale;
  return true;
}
