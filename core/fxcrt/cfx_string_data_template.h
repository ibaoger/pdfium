// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_STRING_DATA_TEMPLATE_H_
#define CORE_FXCRT_CFX_STRING_DATA_TEMPLATE_H_

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"
#include "third_party/base/numerics/safe_math.h"

template <typename CharType>
class CFX_StringDataTemplate {
 public:
  static CFX_StringDataTemplate* Create(size_t nLen) {
    ASSERT(nLen > 0);

    // Calculate space needed for the fixed portion of the struct plus the
    // NUL char that is not included in |m_nAllocLength|.
    int overhead =
        offsetof(CFX_StringDataTemplate, m_String) + sizeof(CharType);
    pdfium::base::CheckedNumeric<size_t> nSize = nLen;
    nSize *= sizeof(CharType);
    nSize += overhead;

    // Now round to an 8-byte boundary. We'd expect that this is the minimum
    // granularity of any of the underlying allocators, so there may be cases
    // where we can save a re-alloc when adding a few characters to a string
    // by using this otherwise wasted space.
    nSize += 7;
    nSize &= ~7;
    size_t totalSize = nSize.ValueOrDie();
    size_t usableLen = (totalSize - overhead) / sizeof(CharType);
    ASSERT(usableLen >= nLen);

    void* pData = pdfium::base::PartitionAllocGeneric(
        gStringPartitionAllocator.root(), totalSize, "CFX_StringDataTemplate");
    return new (pData) CFX_StringDataTemplate(nLen, usableLen);
  }

  static CFX_StringDataTemplate* Create(const CFX_StringDataTemplate& other) {
    CFX_StringDataTemplate* result = Create(other.m_nDataLength);
    result->CopyContents(other);
    return result;
  }

  static CFX_StringDataTemplate* Create(const CharType* pStr, size_t nLen) {
    CFX_StringDataTemplate* result = Create(nLen);
    result->CopyContents(pStr, nLen);
    return result;
  }

  void Retain() { ++m_nRefs; }
  void Release() {
    if (--m_nRefs <= 0)
      pdfium::base::PartitionFreeGeneric(gStringPartitionAllocator.root(),
                                         this);
  }

  bool CanOperateInPlace(size_t nTotalLen) const {
    return m_nRefs <= 1 && nTotalLen <= m_nAllocLength;
  }

  void CopyContents(const CFX_StringDataTemplate& other) {
    ASSERT(other.m_nDataLength <= m_nAllocLength);
    memcpy(m_String, other.m_String,
           (other.m_nDataLength + 1) * sizeof(CharType));
  }

  void CopyContents(const CharType* pStr, size_t nLen) {
    ASSERT(nLen >= 0 && nLen <= m_nAllocLength);
    memcpy(m_String, pStr, nLen * sizeof(CharType));
    m_String[nLen] = 0;
  }

  void CopyContentsAt(size_t offset, const CharType* pStr, size_t nLen) {
    ASSERT(offset >= 0 && nLen >= 0 && offset + nLen <= m_nAllocLength);
    memcpy(m_String + offset, pStr, nLen * sizeof(CharType));
    m_String[offset + nLen] = 0;
  }

  // To ensure ref counts do not overflow, consider the worst possible case:
  // the entire address space contains nothing but pointers to this object.
  // Since the count increments with each new pointer, the largest value is
  // the number of pointers that can fit into the address space. The size of
  // the address space itself is a good upper bound on it.
  intptr_t m_nRefs;

  // These lengths are in terms of number of characters, not bytes, and do not
  // include the terminating NUL character, but the underlying buffer is sized
  // to be capable of holding it.
  size_t m_nDataLength;
  size_t m_nAllocLength;

  // Not really 1, variable size.
  CharType m_String[1];

 private:
  CFX_StringDataTemplate(size_t dataLen, size_t allocLen)
      : m_nRefs(0), m_nDataLength(dataLen), m_nAllocLength(allocLen) {
    ASSERT(dataLen >= 0);
    ASSERT(dataLen <= allocLen);
    m_String[dataLen] = 0;
  }

  ~CFX_StringDataTemplate() = delete;
};

extern template class CFX_StringDataTemplate<char>;
extern template class CFX_StringDataTemplate<wchar_t>;

#endif  // CORE_FXCRT_CFX_STRING_DATA_TEMPLATE_H_
