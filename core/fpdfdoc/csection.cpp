// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/csection.h"

#include <algorithm>

#include "core/fpdfdoc/cline.h"
#include "core/fpdfdoc/cpvt_wordinfo.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

CSection::CSection(CPDF_VariableText* pVT) : m_pVT(pVT) {}

CSection::~CSection() {
  ResetAll();
}

void CSection::ResetAll() {
  ResetWordArray();
  ResetLineArray();
}

void CSection::ResetLineArray() {
  m_LineArray.clear();
}

void CSection::ResetWordArray() {
  for (int32_t i = 0, sz = m_WordArray.GetSize(); i < sz; i++) {
    delete m_WordArray.GetAt(i);
  }
  m_WordArray.RemoveAll();
}

void CSection::ResetLinePlace() {
  int32_t i = 0;
  for (auto& pLine : m_LineArray) {
    if (pLine)
      pLine->LinePlace = CPVT_WordPlace(SecPlace.nSecIndex, i, -1);
    ++i;
  }
}

CPVT_WordPlace CSection::AddWord(const CPVT_WordPlace& place,
                                 const CPVT_WordInfo& wordinfo) {
  CPVT_WordInfo* pWord = new CPVT_WordInfo(wordinfo);
  int32_t nWordIndex =
      std::max(std::min(place.nWordIndex, m_WordArray.GetSize()), 0);
  if (nWordIndex == m_WordArray.GetSize()) {
    m_WordArray.Add(pWord);
  } else {
    m_WordArray.InsertAt(nWordIndex, pWord);
  }
  return place;
}

CPVT_WordPlace CSection::AddLine(const CPVT_LineInfo& lineinfo) {
  m_LineArray.push_back(pdfium::MakeUnique<CLine>());
  m_LineArray.back()->m_LineInfo = lineinfo;
  return CPVT_WordPlace(SecPlace.nSecIndex, m_LineArray.size() - 1, -1);
}

CPVT_FloatRect CSection::Rearrange() {
  if (m_pVT->m_nCharArray > 0) {
    return CTypeset(this).CharArray();
  }
  return CTypeset(this).Typeset();
}

CFX_SizeF CSection::GetSectionSize(float fFontSize) {
  return CTypeset(this).GetEditSize(fFontSize);
}

CPVT_WordPlace CSection::GetBeginWordPlace() const {
  if (!m_LineArray.empty() && m_LineArray.front())
    return m_LineArray.front()->GetBeginWordPlace();
  return SecPlace;
}

CPVT_WordPlace CSection::GetEndWordPlace() const {
  if (!m_LineArray.empty() && m_LineArray.back())
    return m_LineArray.back()->GetEndWordPlace();
  return SecPlace;
}

CPVT_WordPlace CSection::GetPrevWordPlace(const CPVT_WordPlace& place) const {
  if (place.nLineIndex < 0)
    return GetBeginWordPlace();

  if (place.nLineIndex >= pdfium::CollectionSize<int32_t>(m_LineArray))
    return GetEndWordPlace();

  CLine* pLine = m_LineArray[place.nLineIndex].get();
  if (!pLine)
    return place;

  if (place.nWordIndex == pLine->m_LineInfo.nBeginWordIndex)
    return CPVT_WordPlace(place.nSecIndex, place.nLineIndex, -1);

  if (place.nWordIndex >= pLine->m_LineInfo.nBeginWordIndex)
    return pLine->GetPrevWordPlace(place);

  if (!pdfium::IndexInBounds(m_LineArray, place.nLineIndex - 1))
    return place;

  CLine* pPrevLine = m_LineArray[place.nLineIndex - 1].get();
  if (pPrevLine)
    return place;

  return pPrevLine->GetEndWordPlace();
}

CPVT_WordPlace CSection::GetNextWordPlace(const CPVT_WordPlace& place) const {
  if (place.nLineIndex < 0)
    return GetBeginWordPlace();

  if (place.nLineIndex >= pdfium::CollectionSize<int32_t>(m_LineArray))
    return GetEndWordPlace();

  CLine* pLine = m_LineArray[place.nLineIndex].get();
  if (!pLine)
    return place;

  if (place.nWordIndex < pLine->m_LineInfo.nEndWordIndex)
    return pLine->GetNextWordPlace(place);

  if (!pdfium::IndexInBounds(m_LineArray, place.nLineIndex + 1))
    return place;

  CLine* pNextLine = m_LineArray[place.nLineIndex + 1].get();
  if (!pNextLine)
    return place;

  return pNextLine->GetBeginWordPlace();
}

void CSection::UpdateWordPlace(CPVT_WordPlace& place) const {
  int32_t nLeft = 0;
  int32_t nRight = pdfium::CollectionSize<int32_t>(m_LineArray) - 1;
  int32_t nMid = (nLeft + nRight) / 2;
  while (nLeft <= nRight) {
    if (CLine* pLine = m_LineArray[nMid].get()) {
      if (place.nWordIndex < pLine->m_LineInfo.nBeginWordIndex) {
        nRight = nMid - 1;
        nMid = (nLeft + nRight) / 2;
      } else if (place.nWordIndex > pLine->m_LineInfo.nEndWordIndex) {
        nLeft = nMid + 1;
        nMid = (nLeft + nRight) / 2;
      } else {
        place.nLineIndex = nMid;
        return;
      }
    } else {
      break;
    }
  }
}

CPVT_WordPlace CSection::SearchWordPlace(const CFX_PointF& point) const {
  ASSERT(m_pVT);
  CPVT_WordPlace place = GetBeginWordPlace();
  bool bUp = true;
  bool bDown = true;
  int32_t nLeft = 0;
  int32_t nRight = pdfium::CollectionSize<int32_t>(m_LineArray) - 1;
  int32_t nMid = pdfium::CollectionSize<int32_t>(m_LineArray) / 2;
  float fTop = 0;
  float fBottom = 0;
  while (nLeft <= nRight) {
    if (CLine* pLine = m_LineArray[nMid].get()) {
      fTop = pLine->m_LineInfo.fLineY - pLine->m_LineInfo.fLineAscent -
             m_pVT->GetLineLeading(m_SecInfo);
      fBottom = pLine->m_LineInfo.fLineY - pLine->m_LineInfo.fLineDescent;
      if (IsFloatBigger(point.y, fTop))
        bUp = false;
      if (IsFloatSmaller(point.y, fBottom))
        bDown = false;
      if (IsFloatSmaller(point.y, fTop)) {
        nRight = nMid - 1;
        nMid = (nLeft + nRight) / 2;
        continue;
      }
      if (IsFloatBigger(point.y, fBottom)) {
        nLeft = nMid + 1;
        nMid = (nLeft + nRight) / 2;
        continue;
      }
      place = SearchWordPlace(
          point.x,
          CPVT_WordRange(pLine->GetNextWordPlace(pLine->GetBeginWordPlace()),
                         pLine->GetEndWordPlace()));
      place.nLineIndex = nMid;
      return place;
    }
  }
  if (bUp) {
    place = GetBeginWordPlace();
  }
  if (bDown) {
    place = GetEndWordPlace();
  }
  return place;
}

CPVT_WordPlace CSection::SearchWordPlace(
    float fx,
    const CPVT_WordPlace& lineplace) const {
  if (!pdfium::IndexInBounds(m_LineArray, lineplace.nLineIndex))
    return GetBeginWordPlace();

  CLine* pLine = m_LineArray[lineplace.nLineIndex].get();
  if (!pLine)
    return GetBeginWordPlace();

  return SearchWordPlace(
      fx - m_SecInfo.rcSection.left,
      CPVT_WordRange(pLine->GetNextWordPlace(pLine->GetBeginWordPlace()),
                     pLine->GetEndWordPlace()));
}

CPVT_WordPlace CSection::SearchWordPlace(float fx,
                                         const CPVT_WordRange& range) const {
  CPVT_WordPlace wordplace = range.BeginPos;
  wordplace.nWordIndex = -1;
  if (!m_pVT) {
    return wordplace;
  }
  int32_t nLeft = range.BeginPos.nWordIndex;
  int32_t nRight = range.EndPos.nWordIndex + 1;
  int32_t nMid = (nLeft + nRight) / 2;
  while (nLeft < nRight) {
    if (nMid == nLeft) {
      break;
    }
    if (nMid == nRight) {
      nMid--;
      break;
    }
    if (CPVT_WordInfo* pWord = m_WordArray.GetAt(nMid)) {
      if (fx >
          pWord->fWordX + m_pVT->GetWordWidth(*pWord) * VARIABLETEXT_HALF) {
        nLeft = nMid;
        nMid = (nLeft + nRight) / 2;
        continue;
      } else {
        nRight = nMid;
        nMid = (nLeft + nRight) / 2;
        continue;
      }
    } else {
      break;
    }
  }
  if (CPVT_WordInfo* pWord = m_WordArray.GetAt(nMid)) {
    if (fx > pWord->fWordX + m_pVT->GetWordWidth(*pWord) * VARIABLETEXT_HALF) {
      wordplace.nWordIndex = nMid;
    }
  }
  return wordplace;
}

void CSection::ClearLeftWords(int32_t nWordIndex) {
  for (int32_t i = nWordIndex; i >= 0; i--) {
    delete m_WordArray.GetAt(i);
    m_WordArray.RemoveAt(i);
  }
}

void CSection::ClearRightWords(int32_t nWordIndex) {
  for (int32_t i = m_WordArray.GetSize() - 1; i > nWordIndex; i--) {
    delete m_WordArray.GetAt(i);
    m_WordArray.RemoveAt(i);
  }
}

void CSection::ClearMidWords(int32_t nBeginIndex, int32_t nEndIndex) {
  for (int32_t i = nEndIndex; i > nBeginIndex; i--) {
    delete m_WordArray.GetAt(i);
    m_WordArray.RemoveAt(i);
  }
}

void CSection::ClearWords(const CPVT_WordRange& PlaceRange) {
  CPVT_WordPlace SecBeginPos = GetBeginWordPlace();
  CPVT_WordPlace SecEndPos = GetEndWordPlace();
  if (PlaceRange.BeginPos.WordCmp(SecBeginPos) >= 0) {
    if (PlaceRange.EndPos.WordCmp(SecEndPos) <= 0) {
      ClearMidWords(PlaceRange.BeginPos.nWordIndex,
                    PlaceRange.EndPos.nWordIndex);
    } else {
      ClearRightWords(PlaceRange.BeginPos.nWordIndex);
    }
  } else if (PlaceRange.EndPos.WordCmp(SecEndPos) <= 0) {
    ClearLeftWords(PlaceRange.EndPos.nWordIndex);
  } else {
    ResetWordArray();
  }
}

void CSection::ClearWord(const CPVT_WordPlace& place) {
  delete m_WordArray.GetAt(place.nWordIndex);
  m_WordArray.RemoveAt(place.nWordIndex);
}
