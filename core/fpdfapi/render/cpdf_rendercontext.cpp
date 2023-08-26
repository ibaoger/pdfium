// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_rendercontext.h"

#include <utility>

#include "core/fpdfapi/page/cpdf_pageimagecache.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/page/cpdf_pageobjectholder.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/render/cpdf_progressiverenderer.h"
#include "core/fpdfapi/render/cpdf_renderoptions.h"
#include "core/fpdfapi/render/cpdf_renderstatus.h"
#include "core/fpdfapi/render/cpdf_textrenderer.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "third_party/base/check.h"

CPDF_RenderContext::CPDF_RenderContext(
    CPDF_Document* pDoc,
    RetainPtr<CPDF_Dictionary> pPageResources,
    CPDF_PageImageCache* pPageCache)
    : m_pDocument(pDoc),
      m_pPageResources(std::move(pPageResources)),
      m_pPageCache(pPageCache) {}

CPDF_RenderContext::~CPDF_RenderContext() = default;

void CPDF_RenderContext::GetBackground(CFX_RenderDevice* pDevice,
                                       const CPDF_PageObject* pObj,
                                       const CPDF_RenderOptions* pOptions,
                                       const CFX_Matrix& mtFinal) {
  CHECK(pDevice);
  pDevice->FillRect(FX_RECT(0, 0, pDevice->GetWidth(), pDevice->GetHeight()),
                    0xffffffff);
  Render(pDevice, pObj, pOptions, &mtFinal);
}

void CPDF_RenderContext::AppendLayer(CPDF_PageObjectHolder* pObjectHolder,
                                     const CFX_Matrix& mtObject2Device) {
  m_Layers.emplace_back(pObjectHolder, mtObject2Device);
}

void CPDF_RenderContext::Render(CFX_RenderDevice* pDevice,
                                const CPDF_PageObject* pStopObj,
                                const CPDF_RenderOptions* pOptions,
                                const CFX_Matrix* pLastMatrix) {
  for (auto& layer : m_Layers) {
    CFX_RenderDevice::StateRestorer restorer(pDevice);
    CPDF_RenderStatus status(this, pDevice);
    if (pOptions)
      status.SetOptions(*pOptions);
    status.SetStopObject(pStopObj);
    status.SetTransparency(layer.GetObjectHolder()->GetTransparency());
    CFX_Matrix final_matrix = layer.GetMatrix();
    if (pLastMatrix) {
      final_matrix *= *pLastMatrix;
      status.SetDeviceMatrix(*pLastMatrix);
    }
    status.Initialize(nullptr, nullptr);
    status.RenderObjectList(layer.GetObjectHolder(), final_matrix);
    if (status.GetRenderOptions().GetOptions().bLimitedImageCache) {
      m_pPageCache->CacheOptimization(
          status.GetRenderOptions().GetCacheSizeLimit());
    }
    if (status.IsStopped())
      break;
  }
}

CPDF_RenderContext::Layer::Layer(CPDF_PageObjectHolder* pHolder,
                                 const CFX_Matrix& matrix)
    : m_pObjectHolder(pHolder), m_Matrix(matrix) {}

CPDF_RenderContext::Layer::Layer(const Layer& that) = default;

CPDF_RenderContext::Layer::~Layer() = default;
