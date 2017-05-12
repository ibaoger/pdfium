// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_CFX_RAW_PTR_H_
#define CORE_FXCRT_CFX_RAW_PTR_H_

#include <functional>
#include <memory>
#include <utility>

#include "core/fxcrt/fx_memory.h"

template <class T>
class CFX_RawPtr {
 public:
  explicit CFX_RawPtr(T* pObj) : m_pObj(pObj) {}

  CFX_RawPtr() {}
  CFX_RawPtr(const CFX_RawPtr& that) : CFX_RawPtr(that.Get()) {}
  ~CFX_RawPtr() {
    if (m_pObj) {
      reinterpret_cast<volatile uint8_t*>(m_pObj)[0];
    }
  }

  // Deliberately implicit to allow returning nullptrs.
  CFX_RawPtr(std::nullptr_t ptr) {}

  T* Get() const { return m_pObj; }

  CFX_RawPtr& operator=(T* that) {
    m_pObj = that;
    return *this;
  }

  CFX_RawPtr& operator=(const CFX_RawPtr& that) {
    if (*this != that)
      m_pObj = that.Get();
    return *this;
  }


  bool operator==(const CFX_RawPtr& that) const {
    return Get() == that.Get();
  }
  bool operator!=(const CFX_RawPtr& that) const { return !(*this == that); }

  bool operator<(const CFX_RawPtr& that) const {
    return std::less<T*>()(Get(), that.Get());
  }

  explicit operator bool() const { return !!m_pObj; }
  T& operator*() const { return *m_pObj; }
  T* operator->() const { return m_pObj; }

 private:
  T* m_pObj = nullptr;
};

#endif  // CORE_FXCRT_CFX_RAW_PTR_H_
