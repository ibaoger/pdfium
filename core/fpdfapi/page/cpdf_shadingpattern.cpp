// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_shadingpattern.h"

#include <algorithm>

#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_function.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_stream.h"

namespace {

ShadingType ToShadingType(int type) {
  return (type > static_cast<int>(kInvalidShading) &&
          type < static_cast<int>(kMaxShading))
             ? static_cast<ShadingType>(type)
             : kInvalidShading;
}

}  // namespace

CPDF_ShadingPattern::CPDF_ShadingPattern(CPDF_Document* pDoc,
                                         CPDF_Object* pPatternObj,
                                         bool bShading,
                                         const CFX_Matrix& parentMatrix)
    : CPDF_Pattern(pDoc, bShading ? nullptr : pPatternObj, parentMatrix),
      m_ShadingType(kInvalidShading),
      m_bShadingObj(bShading),
      m_pShadingObj(pPatternObj),
      m_pCS(nullptr),
      m_pCountedCS(nullptr) {
  assert(document());
  if (!bShading) {
    m_pShadingObj = pattern_obj()->GetDict()->GetDirectObjectFor("Shading");
    SetPatternToFormMatrix();
  }
}

CPDF_ShadingPattern::~CPDF_ShadingPattern() {
  CPDF_ColorSpace* pCountedCS = m_pCountedCS ? m_pCountedCS->get() : nullptr;
  if (pCountedCS) {
    auto* pPageData = document()->GetPageData();
    if (pPageData) {
      m_pCS.Release();  // Give up unowned reference first.
      pPageData->ReleaseColorSpace(pCountedCS->GetArray());
    }
  }
}

CPDF_TilingPattern* CPDF_ShadingPattern::AsTilingPattern() {
  return nullptr;
}

CPDF_ShadingPattern* CPDF_ShadingPattern::AsShadingPattern() {
  return this;
}

bool CPDF_ShadingPattern::Load() {
  if (m_ShadingType != kInvalidShading)
    return true;

  CPDF_Dictionary* pShadingDict =
      m_pShadingObj ? m_pShadingObj->GetDict() : nullptr;
  if (!pShadingDict)
    return false;

  m_pFunctions.clear();
  CPDF_Object* pFunc = pShadingDict->GetDirectObjectFor("Function");
  if (pFunc) {
    if (CPDF_Array* pArray = pFunc->AsArray()) {
      m_pFunctions.resize(std::min<size_t>(pArray->GetCount(), 4));
      for (size_t i = 0; i < m_pFunctions.size(); ++i)
        m_pFunctions[i] = CPDF_Function::Load(pArray->GetDirectObjectAt(i));
    } else {
      m_pFunctions.push_back(CPDF_Function::Load(pFunc));
    }
  }
  CPDF_Object* pCSObj = pShadingDict->GetDirectObjectFor("ColorSpace");
  if (!pCSObj)
    return false;

  CPDF_DocPageData* pDocPageData = document()->GetPageData();
  m_pCS = pDocPageData->GetColorSpace(pCSObj, nullptr);
  if (m_pCS) {
    // The color space cannot be a Pattern space, according to the PDF 1.7 spec,
    // page 305.
    if (m_pCS->GetFamily() == PDFCS_PATTERN)
      return false;

    m_pCountedCS = pDocPageData->FindColorSpacePtr(m_pCS->GetArray());
  }

  m_ShadingType = ToShadingType(pShadingDict->GetIntegerFor("ShadingType"));

  // We expect to have a stream if our shading type is a mesh.
  if (IsMeshShading() && !ToStream(m_pShadingObj.Get()))
    return false;

  if (!Validate())
    return false;

  return true;
}

bool CPDF_ShadingPattern::Validate() const {
  // Constraints in PDF 1.7 spec, 4.6.3 Shading Patterns, pages 308-331.

  if (m_ShadingType == kInvalidShading || m_ShadingType == kMaxShading)
    return false;

  // Validate color space
  switch (m_ShadingType) {
    case kFunctionBasedShading:
    case kAxialShading:
    case kRadialShading: {
      if (m_pCS->GetFamily() == PDFCS_INDEXED)
        return false;
      break;
    }
    case kFreeFormGouraudTriangleMeshShading:
    case kLatticeFormGouraudTriangleMeshShading:
    case kCoonsPatchMeshShading:
    case kTensorProductPatchMeshShading: {
      if (m_pFunctions.size() > 0 && m_pCS->GetFamily() == PDFCS_INDEXED)
        return false;
      break;
    }
    default: {
      NOTREACHED();
      return false;
    }
  }

  uint32_t numColorSpaceComponents = m_pCS->CountComponents();
  switch (m_ShadingType) {
    case kFunctionBasedShading: {
      // Either one 2-to-N function or N 2-to-1 functions.
      if (!ValidateFunctions(1, 2, numColorSpaceComponents) &&
          !ValidateFunctions(numColorSpaceComponents, 2, 1)) {
        return false;
      }
      break;
    }
    case kAxialShading:
    case kRadialShading: {
      // Either one 1-to-N function or N 1-to-1 functions.
      if (!ValidateFunctions(1, 1, numColorSpaceComponents) &&
          !ValidateFunctions(numColorSpaceComponents, 1, 1)) {
        return false;
      }
      break;
    }
    case kFreeFormGouraudTriangleMeshShading:
    case kLatticeFormGouraudTriangleMeshShading:
    case kCoonsPatchMeshShading:
    case kTensorProductPatchMeshShading: {
      // Either no function, one 1-to-N function, or N 1-to-1 functions.
      if (m_pFunctions.size() > 0 &&
          !ValidateFunctions(1, 1, numColorSpaceComponents) &&
          !ValidateFunctions(numColorSpaceComponents, 1, 1)) {
        return false;
      }
      break;
    }
    default: {
      NOTREACHED();
      return false;
    }
  }
  return true;
}

bool CPDF_ShadingPattern::ValidateFunctions(uint32_t expectedNumFunctions,
                                            uint32_t expectedNumInputs,
                                            uint32_t expectedNumOutputs) const {
  if (m_pFunctions.size() != expectedNumFunctions)
    return false;

  for (auto& function : m_pFunctions) {
    if (function->CountInputs() != expectedNumInputs ||
        function->CountOutputs() != expectedNumOutputs) {
      return false;
    }
  }

  return true;
}
