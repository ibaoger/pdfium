// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPVT_GENERATEAP_H_
#define CORE_FPDFDOC_CPVT_GENERATEAP_H_

#include <memory>

#include "core/fpdfdoc/cpdf_defaultappearance.h"
#include "core/fpdfdoc/cpdf_variabletext.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_color.h"

class CPDF_Dictionary;
class CPDF_Document;
struct CPVT_Dash;
class IPVT_FontMap;

class CPVT_GenerateAP {
 public:
  static bool GenerateCircleAP(CPDF_Document* pDoc,
                               CPDF_Dictionary* pAnnotDict);
  static bool GenerateComboBoxAP(CPDF_Document* pDoc,
                                 CPDF_Dictionary* pAnnotDict);
  static bool GenerateHighlightAP(CPDF_Document* pDoc,
                                  CPDF_Dictionary* pAnnotDict);
  static bool GenerateInkAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict);
  static bool GenerateListBoxAP(CPDF_Document* pDoc,
                                CPDF_Dictionary* pAnnotDict);
  static bool GeneratePopupAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict);
  static bool GenerateSquareAP(CPDF_Document* pDoc,
                               CPDF_Dictionary* pAnnotDict);
  static bool GenerateSquigglyAP(CPDF_Document* pDoc,
                                 CPDF_Dictionary* pAnnotDict);
  static bool GenerateStrikeOutAP(CPDF_Document* pDoc,
                                  CPDF_Dictionary* pAnnotDict);
  static bool GenerateTextAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict);
  static bool GenerateTextFieldAP(CPDF_Document* pDoc,
                                  CPDF_Dictionary* pAnnotDict);
  static bool GenerateUnderlineAP(CPDF_Document* pDoc,
                                  CPDF_Dictionary* pAnnotDict);
  static ByteString GenerateEditAP(IPVT_FontMap* pFontMap,
                                   CPDF_VariableText::Iterator* pIterator,
                                   const CFX_PointF& ptOffset,
                                   bool bContinuous,
                                   uint16_t SubWord);
  static ByteString GenerateBorderAP(const CFX_FloatRect& rect,
                                     float fWidth,
                                     const CFX_Color& color,
                                     const CFX_Color& crLeftTop,
                                     const CFX_Color& crRightBottom,
                                     BorderStyle nStyle,
                                     const CPVT_Dash& dash);
  static ByteString GenerateColorAP(const CFX_Color& color,
                                    PaintOperation nOperation);
  static std::unique_ptr<CPDF_Dictionary> GenerateExtGStateDict(
      const CPDF_Dictionary& pAnnotDict,
      const ByteString& sExtGSDictName,
      const ByteString& sBlendMode);
  static std::unique_ptr<CPDF_Dictionary> GenerateResourceDict(
      CPDF_Document* pDoc,
      std::unique_ptr<CPDF_Dictionary> pExtGStateDict,
      std::unique_ptr<CPDF_Dictionary> pResourceFontDict);
  static void GenerateAndSetAPDict(
      CPDF_Document* pDoc,
      CPDF_Dictionary* pAnnotDict,
      std::ostringstream* psAppStream,
      std::unique_ptr<CPDF_Dictionary> pResourceDict,
      bool bIsTextMarkupAnnotation);
};

#endif  // CORE_FPDFDOC_CPVT_GENERATEAP_H_
