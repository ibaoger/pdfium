// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_blockbuffer.h"

#include <algorithm>
#include <utility>

#include "third_party/base/stl_util.h"

CFX_BlockBuffer::CFX_BlockBuffer()
    : m_iDataLength(0),
      m_iBufferSize(0),
      m_iAllocStep(1024 * 1024),
      m_iStartPosition(0) {}

CFX_BlockBuffer::~CFX_BlockBuffer() {
  ClearBuffer();
}

wchar_t* CFX_BlockBuffer::GetAvailableBlock(int32_t* iIndexInBlock) {
  *iIndexInBlock = 0;
  if (m_BlockArray.empty())
    return nullptr;

  int32_t iRealIndex = m_iStartPosition + m_iDataLength;
  if (iRealIndex == m_iBufferSize) {
    m_BlockArray.emplace_back(FX_Alloc(wchar_t, m_iAllocStep));
    m_iBufferSize += m_iAllocStep;
    return m_BlockArray.back().get();
  }
  *iIndexInBlock = iRealIndex % m_iAllocStep;
  return m_BlockArray[iRealIndex / m_iAllocStep].get();
}

bool CFX_BlockBuffer::InitBuffer() {
  ClearBuffer();
  int32_t iNumOfBlock = (1024 * 1024 - 1) / m_iAllocStep + 1;
  for (int32_t i = 0; i < iNumOfBlock; i++)
    m_BlockArray.emplace_back(FX_Alloc(wchar_t, m_iAllocStep));

  m_iBufferSize = iNumOfBlock * m_iAllocStep;
  return true;
}

void CFX_BlockBuffer::SetTextChar(int32_t iIndex, wchar_t ch) {
  if (iIndex < 0)
    return;

  int32_t iRealIndex = m_iStartPosition + iIndex;
  int32_t iBlockIndex = iRealIndex / m_iAllocStep;
  int32_t iInnerIndex = iRealIndex % m_iAllocStep;
  int32_t iBlockSize = pdfium::CollectionSize<int32_t>(m_BlockArray);
  if (iBlockIndex >= iBlockSize) {
    int32_t iNewBlocks = iBlockIndex - iBlockSize + 1;
    do {
      m_BlockArray.emplace_back(FX_Alloc(wchar_t, m_iAllocStep));
      m_iBufferSize += m_iAllocStep;
    } while (--iNewBlocks);
  }
  wchar_t* pTextData = m_BlockArray[iBlockIndex].get();
  pTextData[iInnerIndex] = ch;
  m_iDataLength = std::max(m_iDataLength, iIndex + 1);
}

int32_t CFX_BlockBuffer::DeleteTextChars(int32_t iCount) {
  if (iCount <= 0)
    return m_iDataLength;

  if (iCount >= m_iDataLength) {
    Reset(false);
    return 0;
  }
  m_iDataLength -= iCount;
  return m_iDataLength;
}

CFX_WideString CFX_BlockBuffer::GetTextData(int32_t iStart,
                                            int32_t iLength) const {
  int32_t iMaybeDataLength = m_iBufferSize - 1 - m_iStartPosition;
  if (iStart < 0 || iStart > iMaybeDataLength)
    return CFX_WideString();
  if (iLength == -1 || iLength > iMaybeDataLength)
    iLength = iMaybeDataLength;
  if (iLength <= 0)
    return CFX_WideString();

  CFX_WideString wsTextData;
  wchar_t* pBuf = wsTextData.GetBuffer(iLength);
  if (!pBuf)
    return CFX_WideString();

  int32_t iStartBlock = 0;
  int32_t iStartInner = 0;
  std::tie(iStartBlock, iStartInner) = TextDataIndex2BufIndex(iStart);

  int32_t iEndBlock = 0;
  int32_t iEndInner = 0;
  std::tie(iEndBlock, iEndInner) = TextDataIndex2BufIndex(iStart + iLength);

  int32_t iPointer = 0;
  for (int32_t i = iStartBlock; i <= iEndBlock; i++) {
    int32_t iBufferPointer = 0;
    int32_t iCopyLength = m_iAllocStep;
    if (i == iStartBlock) {
      iCopyLength -= iStartInner;
      iBufferPointer = iStartInner;
    }
    if (i == iEndBlock)
      iCopyLength -= ((m_iAllocStep - 1) - iEndInner);

    wchar_t* pBlockBuf = m_BlockArray[i].get();
    memcpy(pBuf + iPointer, pBlockBuf + iBufferPointer,
           iCopyLength * sizeof(wchar_t));
    iPointer += iCopyLength;
  }
  wsTextData.ReleaseBuffer(iLength);
  return wsTextData;
}

std::tuple<int32_t, int32_t> CFX_BlockBuffer::TextDataIndex2BufIndex(
    const int32_t iIndex) const {
  ASSERT(iIndex >= 0);

  int32_t iRealIndex = m_iStartPosition + iIndex;
  return std::make_pair(iRealIndex / m_iAllocStep, iRealIndex % m_iAllocStep);
}

void CFX_BlockBuffer::ClearBuffer() {
  m_iBufferSize = 0;
  m_BlockArray.clear();
}
