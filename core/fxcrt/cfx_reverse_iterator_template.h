// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_CFX_REVERSE_ITERATOR_TEMPLATE_H_
#define CORE_FXCRT_CFX_REVERSE_ITERATOR_TEMPLATE_H_

#include <functional>

template <typename CharType>
class CFX_ReverseIterator {
 public:
  explicit CFX_ReverseIterator(const CharType* ptr) : m_Ptr(ptr) {}
  CFX_ReverseIterator(const CFX_ReverseIterator& that) : m_Ptr(that.m_Ptr) {}

  const CharType& operator*() const { return *(m_Ptr - 1); }
  const CFX_ReverseIterator& operator++() {  // Preincrement.
    --m_Ptr;
    return *this;
  }
  const CFX_ReverseIterator operator++(int) {  // Post, by convention.
    CFX_ReverseIterator old(*this);
    --m_Ptr;
    return old;
  }
  const CFX_ReverseIterator& operator--() {  // Preincrement.
    ++m_Ptr;
    return *this;
  }
  const CFX_ReverseIterator operator--(int) {  // Post, by convention.
    CFX_ReverseIterator old(*this);
    ++m_Ptr;
    return old;
  }
  bool operator==(const CFX_ReverseIterator& that) const {
    return m_Ptr == that.m_Ptr;
  }
  bool operator!=(const CFX_ReverseIterator& that) const {
    return !(*this == that);
  }
  bool operator<(const CFX_ReverseIterator& that) const {
    return std::less<const CharType*>()(that.m_Ptr, m_Ptr);
  }

 private:
  const CharType* m_Ptr;  // Current character is one before ptr.
};

#endif  // CORE_FXCRT_CFX_REVERSE_ITERATOR_TEMPLATE_H_
