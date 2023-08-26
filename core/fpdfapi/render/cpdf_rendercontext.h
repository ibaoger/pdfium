// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_RENDERCONTEXT_H_
#define CORE_FPDFAPI_RENDER_CPDF_RENDERCONTEXT_H_

#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CFX_Matrix;
class CFX_RenderDevice;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_PageImageCache;
class CPDF_PageObject;
class CPDF_PageObjectHolder;
class CPDF_RenderOptions;

class CPDF_RenderContext {
 public:
  class Layer {
   public:
    Layer(CPDF_PageObjectHolder* pHolder, const CFX_Matrix& matrix);
    Layer(const Layer& that);
    ~Layer();

    CPDF_PageObjectHolder* GetObjectHolder() { return m_pObjectHolder; }
    const CFX_Matrix& GetMatrix() const { return m_Matrix; }

   private:
    UnownedPtr<CPDF_PageObjectHolder> const m_pObjectHolder;
    const CFX_Matrix m_Matrix;
  };

  CPDF_RenderContext(CPDF_Document* pDoc,
                     RetainPtr<CPDF_Dictionary> pPageResources,
                     CPDF_PageImageCache* pPageCache);
  ~CPDF_RenderContext();

  void AppendLayer(CPDF_PageObjectHolder* pObjectHolder,
                   const CFX_Matrix& mtObject2Device);

  void Render(CFX_RenderDevice* pDevice,
              const CPDF_PageObject* pStopObj,
              const CPDF_RenderOptions* pOptions,
              const CFX_Matrix* pLastMatrix);

  void GetBackground(CFX_RenderDevice* pDevice,
                     const CPDF_PageObject* pObj,
                     const CPDF_RenderOptions* pOptions,
                     const CFX_Matrix& mtFinal);

  size_t CountLayers() const { return m_Layers.size(); }
  Layer* GetLayer(uint32_t index) { return &m_Layers[index]; }

  CPDF_Document* GetDocument() const { return m_pDocument; }
  const CPDF_Dictionary* GetPageResources() const {
    return m_pPageResources.Get();
  }
  RetainPtr<CPDF_Dictionary> GetMutablePageResources() {
    return m_pPageResources;
  }
  CPDF_PageImageCache* GetPageCache() const { return m_pPageCache; }

 private:
  UnownedPtr<CPDF_Document> const m_pDocument;
  RetainPtr<CPDF_Dictionary> const m_pPageResources;
  UnownedPtr<CPDF_PageImageCache> const m_pPageCache;
  std::vector<Layer> m_Layers;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_RENDERCONTEXT_H_
