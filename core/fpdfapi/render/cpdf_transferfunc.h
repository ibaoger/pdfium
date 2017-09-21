// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_TRANSFERFUNC_H_
#define CORE_FPDFAPI_RENDER_CPDF_TRANSFERFUNC_H_

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/fx_dib.h"

class CPDF_Document;
class CFX_DIBSource;

class CPDF_TransferFunc : public Retainable {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  FX_COLORREF TranslateColor(FX_COLORREF src) const;
  RetainPtr<CFX_DIBSource> TranslateImage(const RetainPtr<CFX_DIBSource>& pSrc);

  UnownedPtr<CPDF_Document> const m_pPDFDoc;
  bool m_bIdentity;
  uint8_t m_Samples[256 * 3];

 private:
  explicit CPDF_TransferFunc(CPDF_Document* pDoc);
  ~CPDF_TransferFunc() override;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_TRANSFERFUNC_H_
