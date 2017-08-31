// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_bidi.h"

#include <algorithm>

#include "core/fxcrt/fx_ucd.h"
#include "third_party/base/ptr_util.h"

#ifdef PDF_ENABLE_XFA
#include "core/fxcrt/fx_extension.h"

namespace {

constexpr int32_t kBidiMaxLevel = 61;

enum FX_BIDIWEAKSTATE {
  FX_BWSxa = 0,
  FX_BWSxr,
  FX_BWSxl,
  FX_BWSao,
  FX_BWSro,
  FX_BWSlo,
  FX_BWSrt,
  FX_BWSlt,
  FX_BWScn,
  FX_BWSra,
  FX_BWSre,
  FX_BWSla,
  FX_BWSle,
  FX_BWSac,
  FX_BWSrc,
  FX_BWSrs,
  FX_BWSlc,
  FX_BWSls,
  FX_BWSret,
  FX_BWSlet
};

enum FX_BIDIWEAKACTION {
  FX_BWAIX = 0x100,
  FX_BWAXX = 0x0F,
  FX_BWAxxx = (0x0F << 4) + 0x0F,
  FX_BWAxIx = 0x100 + FX_BWAxxx,
  FX_BWAxxN = (0x0F << 4) + FX_BIDICLASS_ON,
  FX_BWAxxE = (0x0F << 4) + FX_BIDICLASS_EN,
  FX_BWAxxA = (0x0F << 4) + FX_BIDICLASS_AN,
  FX_BWAxxR = (0x0F << 4) + FX_BIDICLASS_R,
  FX_BWAxxL = (0x0F << 4) + FX_BIDICLASS_L,
  FX_BWANxx = (FX_BIDICLASS_ON << 4) + 0x0F,
  FX_BWAAxx = (FX_BIDICLASS_AN << 4) + 0x0F,
  FX_BWAExE = (FX_BIDICLASS_EN << 4) + FX_BIDICLASS_EN,
  FX_BWANIx = (FX_BIDICLASS_ON << 4) + 0x0F + 0x100,
  FX_BWANxN = (FX_BIDICLASS_ON << 4) + FX_BIDICLASS_ON,
  FX_BWANxR = (FX_BIDICLASS_ON << 4) + FX_BIDICLASS_R,
  FX_BWANxE = (FX_BIDICLASS_ON << 4) + FX_BIDICLASS_EN,
  FX_BWAAxA = (FX_BIDICLASS_AN << 4) + FX_BIDICLASS_AN,
  FX_BWANxL = (FX_BIDICLASS_ON << 4) + FX_BIDICLASS_L,
  FX_BWALxL = (FX_BIDICLASS_L << 4) + FX_BIDICLASS_L,
  FX_BWAxIL = (0x0F << 4) + FX_BIDICLASS_L + 0x100,
  FX_BWAAxR = (FX_BIDICLASS_AN << 4) + FX_BIDICLASS_R,
  FX_BWALxx = (FX_BIDICLASS_L << 4) + 0x0F,
};

enum FX_BIDINEUTRALSTATE {
  FX_BNSr = 0,
  FX_BNSl,
  FX_BNSrn,
  FX_BNSln,
  FX_BNSa,
  FX_BNSna
};

enum FX_BIDINEUTRALACTION {
  FX_BNAnL = FX_BIDICLASS_L,
  FX_BNAEn = (FX_BIDICLASS_AN << 4),
  FX_BNARn = (FX_BIDICLASS_R << 4),
  FX_BNALn = (FX_BIDICLASS_L << 4),
  FX_BNAIn = FX_BWAIX,
  FX_BNALnL = (FX_BIDICLASS_L << 4) + FX_BIDICLASS_L,
};

const int32_t gc_FX_BidiNTypes[] = {
    FX_BIDICLASS_N,   FX_BIDICLASS_L,   FX_BIDICLASS_R,   FX_BIDICLASS_AN,
    FX_BIDICLASS_EN,  FX_BIDICLASS_AL,  FX_BIDICLASS_NSM, FX_BIDICLASS_CS,
    FX_BIDICLASS_ES,  FX_BIDICLASS_ET,  FX_BIDICLASS_BN,  FX_BIDICLASS_BN,
    FX_BIDICLASS_N,   FX_BIDICLASS_B,   FX_BIDICLASS_RLO, FX_BIDICLASS_RLE,
    FX_BIDICLASS_LRO, FX_BIDICLASS_LRE, FX_BIDICLASS_PDF, FX_BIDICLASS_ON,
};

const int32_t gc_FX_BidiWeakStates[][10] = {
    {FX_BWSao, FX_BWSxl, FX_BWSxr, FX_BWScn, FX_BWScn, FX_BWSxa, FX_BWSxa,
     FX_BWSao, FX_BWSao, FX_BWSao},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSxr,
     FX_BWSro, FX_BWSro, FX_BWSrt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSxl,
     FX_BWSlo, FX_BWSlo, FX_BWSlt},
    {FX_BWSao, FX_BWSxl, FX_BWSxr, FX_BWScn, FX_BWScn, FX_BWSxa, FX_BWSao,
     FX_BWSao, FX_BWSao, FX_BWSao},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSro,
     FX_BWSro, FX_BWSro, FX_BWSrt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSlo,
     FX_BWSlo, FX_BWSlo, FX_BWSlt},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSrt,
     FX_BWSro, FX_BWSro, FX_BWSrt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSlt,
     FX_BWSlo, FX_BWSlo, FX_BWSlt},
    {FX_BWSao, FX_BWSxl, FX_BWSxr, FX_BWScn, FX_BWScn, FX_BWSxa, FX_BWScn,
     FX_BWSac, FX_BWSao, FX_BWSao},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSra,
     FX_BWSrc, FX_BWSro, FX_BWSrt},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSre,
     FX_BWSrs, FX_BWSrs, FX_BWSret},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSla,
     FX_BWSlc, FX_BWSlo, FX_BWSlt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSle,
     FX_BWSls, FX_BWSls, FX_BWSlet},
    {FX_BWSao, FX_BWSxl, FX_BWSxr, FX_BWScn, FX_BWScn, FX_BWSxa, FX_BWSao,
     FX_BWSao, FX_BWSao, FX_BWSao},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSro,
     FX_BWSro, FX_BWSro, FX_BWSrt},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSro,
     FX_BWSro, FX_BWSro, FX_BWSrt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSlo,
     FX_BWSlo, FX_BWSlo, FX_BWSlt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSlo,
     FX_BWSlo, FX_BWSlo, FX_BWSlt},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSret,
     FX_BWSro, FX_BWSro, FX_BWSret},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSlet,
     FX_BWSlo, FX_BWSlo, FX_BWSlet},
};

const int32_t gc_FX_BidiWeakActions[][10] = {
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxA, FX_BWAxxR,
     FX_BWAxxR, FX_BWAxxN, FX_BWAxxN, FX_BWAxxN},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxE, FX_BWAxxR,
     FX_BWAxxR, FX_BWAxxN, FX_BWAxxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxL, FX_BWAxxR,
     FX_BWAxxL, FX_BWAxxN, FX_BWAxxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxA, FX_BWAxxR,
     FX_BWAxxN, FX_BWAxxN, FX_BWAxxN, FX_BWAxxN},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxE, FX_BWAxxR,
     FX_BWAxxN, FX_BWAxxN, FX_BWAxxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxL, FX_BWAxxR,
     FX_BWAxxN, FX_BWAxxN, FX_BWAxxN, FX_BWAxIx},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWAExE, FX_BWANxR,
     FX_BWAxIx, FX_BWANxN, FX_BWANxN, FX_BWAxIx},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWALxL, FX_BWANxR,
     FX_BWAxIx, FX_BWANxN, FX_BWANxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxA, FX_BWAxxR,
     FX_BWAxxA, FX_BWAxIx, FX_BWAxxN, FX_BWAxxN},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxE, FX_BWAxxR,
     FX_BWAxxA, FX_BWAxIx, FX_BWAxxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxE, FX_BWAxxR,
     FX_BWAxxE, FX_BWAxIx, FX_BWAxIx, FX_BWAxxE},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxL, FX_BWAxxR,
     FX_BWAxxA, FX_BWAxIx, FX_BWAxxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxL, FX_BWAxxR,
     FX_BWAxxL, FX_BWAxIx, FX_BWAxIx, FX_BWAxxL},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWAAxx, FX_BWAAxA, FX_BWANxR,
     FX_BWANxN, FX_BWANxN, FX_BWANxN, FX_BWANxN},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWAAxx, FX_BWANxE, FX_BWANxR,
     FX_BWANxN, FX_BWANxN, FX_BWANxN, FX_BWANIx},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWAExE, FX_BWANxR,
     FX_BWANxN, FX_BWANxN, FX_BWANxN, FX_BWANIx},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWAAxx, FX_BWANxL, FX_BWANxR,
     FX_BWANxN, FX_BWANxN, FX_BWANxN, FX_BWANIx},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWALxL, FX_BWANxR,
     FX_BWANxN, FX_BWANxN, FX_BWANxN, FX_BWANIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxE, FX_BWAxxR,
     FX_BWAxxE, FX_BWAxxN, FX_BWAxxN, FX_BWAxxE},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxL, FX_BWAxxR,
     FX_BWAxxL, FX_BWAxxN, FX_BWAxxN, FX_BWAxxL},
};

const int32_t gc_FX_BidiNeutralStates[][5] = {
    {FX_BNSrn, FX_BNSl, FX_BNSr, FX_BNSr, FX_BNSr},
    {FX_BNSln, FX_BNSl, FX_BNSr, FX_BNSa, FX_BNSl},
    {FX_BNSrn, FX_BNSl, FX_BNSr, FX_BNSr, FX_BNSr},
    {FX_BNSln, FX_BNSl, FX_BNSr, FX_BNSa, FX_BNSl},
    {FX_BNSna, FX_BNSl, FX_BNSr, FX_BNSa, FX_BNSl},
    {FX_BNSna, FX_BNSl, FX_BNSr, FX_BNSa, FX_BNSl},
};
const int32_t gc_FX_BidiNeutralActions[][5] = {
    {FX_BNAIn, 0, 0, 0, 0},
    {FX_BNAIn, 0, 0, 0, FX_BIDICLASS_L},
    {FX_BNAIn, FX_BNAEn, FX_BNARn, FX_BNARn, FX_BNARn},
    {FX_BNAIn, FX_BNALn, FX_BNAEn, FX_BNAEn, FX_BNALnL},
    {FX_BNAIn, 0, 0, 0, FX_BIDICLASS_L},
    {FX_BNAIn, FX_BNAEn, FX_BNARn, FX_BNARn, FX_BNAEn},
};

const int32_t gc_FX_BidiAddLevel[][4] = {
    {0, 1, 2, 2},
    {1, 0, 1, 1},
};

class CFX_BidiLine {
 public:
  void BidiLine(std::vector<CFX_Char>* chars, int32_t iCount) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    if (iCount < 2)
      return;

    Classify(chars, iCount, false);
    ResolveExplicit(chars, iCount);
    ResolveWeak(chars, iCount);
    ResolveNeutrals(chars, iCount);
    ResolveImplicit(chars, iCount);
    Classify(chars, iCount, true);
    ResolveWhitespace(chars, iCount);
    Reorder(chars, iCount);
    Position(chars, iCount);
  }

 private:
  int32_t Direction(int32_t val) {
    return FX_IsOdd(val) ? FX_BIDICLASS_R : FX_BIDICLASS_L;
  }

  int32_t GetDeferredType(int32_t val) { return (val >> 4) & 0x0F; }

  int32_t GetResolvedType(int32_t val) { return val & 0x0F; }

  int32_t GetDeferredNeutrals(int32_t iAction, int32_t iLevel) {
    iAction = (iAction >> 4) & 0xF;
    if (iAction == (FX_BNAEn >> 4))
      return Direction(iLevel);
    return iAction;
  }

  int32_t GetResolvedNeutrals(int32_t iAction) {
    iAction &= 0xF;
    return iAction == FX_BNAIn ? 0 : iAction;
  }

  void ReverseString(std::vector<CFX_Char>* chars,
                     int32_t iStart,
                     int32_t iCount) {
    ASSERT(pdfium::IndexInBounds(*chars, iStart));
    ASSERT(iCount >= 0 &&
           iStart + iCount <= pdfium::CollectionSize<int32_t>(*chars));
    std::reverse(chars->begin() + iStart, chars->begin() + iStart + iCount);
  }

  void SetDeferredRun(std::vector<CFX_Char>* chars,
                      bool bClass,
                      int32_t iStart,
                      int32_t iCount,
                      int32_t iValue) {
    ASSERT(iStart >= 0 && iStart <= pdfium::CollectionSize<int32_t>(*chars));
    ASSERT(iStart - iCount > -1);
    int32_t iLast = iStart - iCount;
    if (bClass) {
      for (int32_t i = iStart - 1; i >= iLast; i--)
        (*chars)[i].m_iBidiClass = static_cast<int16_t>(iValue);
      return;
    }

    for (int32_t i = iStart - 1; i >= iLast; i--)
      (*chars)[i].m_iBidiLevel = static_cast<int16_t>(iValue);
  }

  void Classify(std::vector<CFX_Char>* chars, int32_t iCount, bool bWS) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    if (bWS) {
      for (int32_t i = 0; i < iCount; i++) {
        CFX_Char& cur = (*chars)[i];
        cur.m_iBidiClass =
            static_cast<int16_t>(cur.char_props() & FX_BIDICLASSBITSMASK) >>
            FX_BIDICLASSBITS;
      }
      return;
    }

    for (int32_t i = 0; i < iCount; i++) {
      CFX_Char& cur = (*chars)[i];
      cur.m_iBidiClass = static_cast<int16_t>(
          gc_FX_BidiNTypes[(cur.char_props() & FX_BIDICLASSBITSMASK) >>
                           FX_BIDICLASSBITS]);
    }
  }

  void ResolveExplicit(std::vector<CFX_Char>* chars, int32_t iCount) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    for (int32_t i = 0; i < iCount; i++)
      (*chars)[i].m_iBidiLevel = 0;
  }

  void ResolveWeak(std::vector<CFX_Char>* chars, int32_t iCount) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    iCount--;
    if (iCount < 1)
      return;

    int32_t iLevelCur = 0;
    int32_t iState = FX_BWSxl;
    int32_t i = 0;
    int32_t iNum = 0;
    int32_t iClsCur;
    int32_t iClsRun;
    int32_t iClsNew;
    int32_t iAction;
    for (; i <= iCount; i++) {
      CFX_Char* pTC = &(*chars)[i];
      iClsCur = pTC->m_iBidiClass;
      if (iClsCur == FX_BIDICLASS_BN) {
        pTC->m_iBidiLevel = (int16_t)iLevelCur;
        if (i == iCount && iLevelCur != 0) {
          iClsCur = Direction(iLevelCur);
          pTC->m_iBidiClass = (int16_t)iClsCur;
        } else if (i < iCount) {
          CFX_Char* pTCNext = &(*chars)[i + 1];
          int32_t iLevelNext, iLevelNew;
          iClsNew = pTCNext->m_iBidiClass;
          iLevelNext = pTCNext->m_iBidiLevel;
          if (iClsNew != FX_BIDICLASS_BN && iLevelCur != iLevelNext) {
            iLevelNew = std::max(iLevelNext, iLevelCur);
            pTC->m_iBidiLevel = static_cast<int16_t>(iLevelNew);
            iClsCur = Direction(iLevelNew);
            pTC->m_iBidiClass = static_cast<int16_t>(iClsCur);
            iLevelCur = iLevelNext;
          } else {
            if (iNum > 0)
              iNum++;
            continue;
          }
        } else {
          if (iNum > 0)
            iNum++;
          continue;
        }
      }

      ASSERT(iClsCur <= FX_BIDICLASS_BN);
      iAction = gc_FX_BidiWeakActions[iState][iClsCur];
      iClsRun = GetDeferredType(iAction);
      if (iClsRun != FX_BWAXX && iNum > 0) {
        SetDeferredRun(chars, true, i, iNum, iClsRun);
        iNum = 0;
      }
      iClsNew = GetResolvedType(iAction);
      if (iClsNew != FX_BWAXX)
        pTC->m_iBidiClass = static_cast<int16_t>(iClsNew);
      if (FX_BWAIX & iAction)
        iNum++;

      iState = gc_FX_BidiWeakStates[iState][iClsCur];
    }
    if (iNum > 0) {
      iClsCur = Direction(0);
      iClsRun = GetDeferredType(gc_FX_BidiWeakActions[iState][iClsCur]);
      if (iClsRun != FX_BWAXX)
        SetDeferredRun(chars, true, i, iNum, iClsRun);
    }
  }

  void ResolveNeutrals(std::vector<CFX_Char>* chars, int32_t iCount) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    iCount--;
    if (iCount < 1)
      return;

    CFX_Char* pTC;
    int32_t iLevel = 0;
    int32_t iState = FX_BNSl;
    int32_t i = 0;
    int32_t iNum = 0;
    int32_t iClsCur;
    int32_t iClsRun;
    int32_t iClsNew;
    int32_t iAction;
    for (; i <= iCount; i++) {
      pTC = &(*chars)[i];
      iClsCur = pTC->m_iBidiClass;
      if (iClsCur == FX_BIDICLASS_BN) {
        if (iNum)
          iNum++;
        continue;
      }

      ASSERT(iClsCur < FX_BIDICLASS_AL);
      iAction = gc_FX_BidiNeutralActions[iState][iClsCur];
      iClsRun = GetDeferredNeutrals(iAction, iLevel);
      if (iClsRun != FX_BIDICLASS_N && iNum > 0) {
        SetDeferredRun(chars, true, i, iNum, iClsRun);
        iNum = 0;
      }

      iClsNew = GetResolvedNeutrals(iAction);
      if (iClsNew != FX_BIDICLASS_N)
        pTC->m_iBidiClass = (int16_t)iClsNew;
      if (FX_BNAIn & iAction)
        iNum++;

      iState = gc_FX_BidiNeutralStates[iState][iClsCur];
      iLevel = pTC->m_iBidiLevel;
    }
    if (iNum > 0) {
      iClsCur = Direction(iLevel);
      iClsRun = GetDeferredNeutrals(gc_FX_BidiNeutralActions[iState][iClsCur],
                                    iLevel);
      if (iClsRun != FX_BIDICLASS_N)
        SetDeferredRun(chars, true, i, iNum, iClsRun);
    }
  }

  void ResolveImplicit(std::vector<CFX_Char>* chars, int32_t iCount) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    for (int32_t i = 0; i < iCount; i++) {
      int32_t iCls = (*chars)[i].m_iBidiClass;
      if (iCls == FX_BIDICLASS_BN)
        continue;

      ASSERT(iCls > FX_BIDICLASS_ON && iCls < FX_BIDICLASS_AL);
      int32_t iLevel = (*chars)[i].m_iBidiLevel;
      iLevel += gc_FX_BidiAddLevel[FX_IsOdd(iLevel)][iCls - 1];
      (*chars)[i].m_iBidiLevel = (int16_t)iLevel;
    }
  }

  void ResolveWhitespace(std::vector<CFX_Char>* chars, int32_t iCount) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    if (iCount < 1)
      return;

    iCount--;
    int32_t iLevel = 0;
    int32_t i = 0;
    int32_t iNum = 0;
    for (; i <= iCount; i++) {
      switch ((*chars)[i].m_iBidiClass) {
        case FX_BIDICLASS_WS:
          iNum++;
          break;
        case FX_BIDICLASS_RLE:
        case FX_BIDICLASS_LRE:
        case FX_BIDICLASS_LRO:
        case FX_BIDICLASS_RLO:
        case FX_BIDICLASS_PDF:
        case FX_BIDICLASS_BN:
          (*chars)[i].m_iBidiLevel = (int16_t)iLevel;
          iNum++;
          break;
        case FX_BIDICLASS_S:
        case FX_BIDICLASS_B:
          if (iNum > 0)
            SetDeferredRun(chars, false, i, iNum, 0);

          (*chars)[i].m_iBidiLevel = 0;
          iNum = 0;
          break;
        default:
          iNum = 0;
          break;
      }
      iLevel = (*chars)[i].m_iBidiLevel;
    }
    if (iNum > 0)
      SetDeferredRun(chars, false, i, iNum, 0);
  }

  int32_t ReorderLevel(std::vector<CFX_Char>* chars,
                       int32_t iCount,
                       int32_t iBaseLevel,
                       int32_t iStart,
                       bool bReverse) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    ASSERT(iBaseLevel >= 0 && iBaseLevel <= kBidiMaxLevel);
    ASSERT(iStart >= 0 && iStart < iCount);

    if (iCount < 1)
      return 0;

    bReverse = bReverse || FX_IsOdd(iBaseLevel);
    int32_t i = iStart;
    for (; i < iCount; i++) {
      int32_t iLevel = (*chars)[i].m_iBidiLevel;
      if (iLevel == iBaseLevel)
        continue;
      if (iLevel < iBaseLevel)
        break;
      i += ReorderLevel(chars, iCount, iBaseLevel + 1, i, bReverse) - 1;
    }
    int32_t iNum = i - iStart;
    if (bReverse && iNum > 1)
      ReverseString(chars, iStart, iNum);

    return iNum;
  }

  void Reorder(std::vector<CFX_Char>* chars, int32_t iCount) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    int32_t i = 0;
    while (i < iCount)
      i += ReorderLevel(chars, iCount, 0, i, false);
  }

  void Position(std::vector<CFX_Char>* chars, int32_t iCount) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    for (int32_t i = 0; i < iCount; ++i)
      (*chars)[(*chars)[i].m_iBidiPos].m_iBidiOrder = i;
  }
};

}  // namespace

#endif  // PDF_ENABLE_XFA

CFX_BidiChar::CFX_BidiChar()
    : m_CurrentSegment({0, 0, NEUTRAL}), m_LastSegment({0, 0, NEUTRAL}) {}

bool CFX_BidiChar::AppendChar(wchar_t wch) {
  uint32_t dwProps = FX_GetUnicodeProperties(wch);
  int32_t iBidiCls = (dwProps & FX_BIDICLASSBITSMASK) >> FX_BIDICLASSBITS;
  Direction direction = NEUTRAL;
  switch (iBidiCls) {
    case FX_BIDICLASS_L:
    case FX_BIDICLASS_AN:
    case FX_BIDICLASS_EN:
      direction = LEFT;
      break;
    case FX_BIDICLASS_R:
    case FX_BIDICLASS_AL:
      direction = RIGHT;
      break;
  }

  bool bChangeDirection = (direction != m_CurrentSegment.direction);
  if (bChangeDirection)
    StartNewSegment(direction);

  m_CurrentSegment.count++;
  return bChangeDirection;
}

bool CFX_BidiChar::EndChar() {
  StartNewSegment(NEUTRAL);
  return m_LastSegment.count > 0;
}

void CFX_BidiChar::StartNewSegment(CFX_BidiChar::Direction direction) {
  m_LastSegment = m_CurrentSegment;
  m_CurrentSegment.start += m_CurrentSegment.count;
  m_CurrentSegment.count = 0;
  m_CurrentSegment.direction = direction;
}

CFX_BidiString::CFX_BidiString(const CFX_WideString& str)
    : m_Str(str),
      m_pBidiChar(pdfium::MakeUnique<CFX_BidiChar>()),
      m_eOverallDirection(CFX_BidiChar::LEFT) {
  for (const auto& c : m_Str) {
    if (m_pBidiChar->AppendChar(c))
      m_Order.push_back(m_pBidiChar->GetSegmentInfo());
  }
  if (m_pBidiChar->EndChar())
    m_Order.push_back(m_pBidiChar->GetSegmentInfo());

  size_t nR2L = std::count_if(m_Order.begin(), m_Order.end(),
                              [](const CFX_BidiChar::Segment& seg) {
                                return seg.direction == CFX_BidiChar::RIGHT;
                              });

  size_t nL2R = std::count_if(m_Order.begin(), m_Order.end(),
                              [](const CFX_BidiChar::Segment& seg) {
                                return seg.direction == CFX_BidiChar::LEFT;
                              });

  if (nR2L > 0 && nR2L >= nL2R)
    SetOverallDirectionRight();
}

CFX_BidiString::~CFX_BidiString() {}

void CFX_BidiString::SetOverallDirectionRight() {
  if (m_eOverallDirection != CFX_BidiChar::RIGHT) {
    std::reverse(m_Order.begin(), m_Order.end());
    m_eOverallDirection = CFX_BidiChar::RIGHT;
  }
}

#ifdef PDF_ENABLE_XFA
void FX_BidiLine(std::vector<CFX_Char>* chars, int32_t iCount) {
  CFX_BidiLine blt;
  blt.BidiLine(chars, iCount);
}
#endif  // PDF_ENABLE_XFA
