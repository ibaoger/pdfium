// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_COUNTEDOBJECT_H_
#define CORE_FPDFAPI_PAGE_CPDF_COUNTEDOBJECT_H_

#include <memory>
#include <utility>

#include "core/fpdfapi/page/cpdf_colorspace.h"
#include "core/fpdfapi/page/cpdf_pattern.h"
#include "core/fxcrt/fx_system.h"

template <class T>
class CPDF_CountedObject {
 public:
  explicit CPDF_CountedObject(std::unique_ptr<T> ptr)
      : m_nCount(1), m_pObj(std::move(ptr)) {}
  void reset(std::unique_ptr<T> ptr) {  // CAUTION: tosses prior ref counts.
    m_nCount = 1;
    m_pObj = std::move(ptr);
  }
  void clear() {  // Now you're all weak ptrs ...
    // Guards against accidental re-entry by nulling m_pObj first.
    std::unique_ptr<T> temp;
    m_pObj.swap(temp);
  }
  T* get() const { return m_pObj.get(); }
  T* AddRef() {
    ASSERT(m_pObj);
    ++m_nCount;
    return m_pObj.get();
  }
  void RemoveRef() {
    if (m_nCount)
      --m_nCount;
  }
  size_t use_count() const { return m_nCount; }

 protected:
  size_t m_nCount;
  std::unique_ptr<T> m_pObj;
};
using CPDF_CountedColorSpace = CPDF_CountedObject<CPDF_ColorSpace>;
using CPDF_CountedPattern = CPDF_CountedObject<CPDF_Pattern>;

#endif  // CORE_FPDFAPI_PAGE_CPDF_COUNTEDOBJECT_H_
