// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_WIDESTRING_H_
#define CORE_FXCRT_CFX_WIDESTRING_H_

#include <functional>
#include <utility>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/cfx_string_c_template.h"
#include "core/fxcrt/cfx_string_data_template.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"
#include "third_party/base/optional.h"

class CFX_ByteString;

// A mutable string with shared buffers using copy-on-write semantics that
// avoids the cost of std::string's iterator stability guarantees.
class CFX_WideString {
 public:
  using CharType = wchar_t;
  using const_iterator = const CharType*;

  CFX_WideString();
  CFX_WideString(const CFX_WideString& other);
  CFX_WideString(CFX_WideString&& other) noexcept;

  // Deliberately implicit to avoid calling on every string literal.
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_WideString(wchar_t ch);
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_WideString(const wchar_t* ptr);

  // No implicit conversions from byte strings.
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_WideString(char) = delete;

  CFX_WideString(const wchar_t* ptr, size_t len);

  explicit CFX_WideString(const CFX_WideStringC& str);
  CFX_WideString(const CFX_WideStringC& str1, const CFX_WideStringC& str2);
  CFX_WideString(const std::initializer_list<CFX_WideStringC>& list);

  ~CFX_WideString();

  static CFX_WideString FromLocal(const CFX_ByteStringC& str);
  static CFX_WideString FromCodePage(const CFX_ByteStringC& str,
                                     uint16_t codepage);

  static CFX_WideString FromUTF8(const CFX_ByteStringC& str);
  static CFX_WideString FromUTF16LE(const unsigned short* str, size_t len);

  static size_t WStringLength(const unsigned short* str);

  // Explicit conversion to C-style wide string.
  // Note: Any subsequent modification of |this| will invalidate the result.
  const wchar_t* c_str() const { return m_pData ? m_pData->m_String : L""; }

  // Explicit conversion to CFX_WideStringC.
  // Note: Any subsequent modification of |this| will invalidate the result.
  CFX_WideStringC AsStringC() const {
    return CFX_WideStringC(c_str(), GetLength());
  }

  // Note: Any subsequent modification of |this| will invalidate iterators.
  const_iterator begin() const { return m_pData ? m_pData->m_String : nullptr; }
  const_iterator end() const {
    return m_pData ? m_pData->m_String + m_pData->m_nDataLength : nullptr;
  }

  void clear() { m_pData.Reset(); }

  size_t GetLength() const { return m_pData ? m_pData->m_nDataLength : 0; }
  size_t GetStringLength() const {
    return m_pData ? FXSYS_wcslen(m_pData->m_String) : 0;
  }
  bool IsEmpty() const { return !GetLength(); }
  bool IsValidIndex(size_t index) const { return index < GetLength(); }
  bool IsValidLength(size_t length) const { return length <= GetLength(); }

  const CFX_WideString& operator=(const wchar_t* str);
  const CFX_WideString& operator=(const CFX_WideString& stringSrc);
  const CFX_WideString& operator=(const CFX_WideStringC& stringSrc);

  const CFX_WideString& operator+=(const wchar_t* str);
  const CFX_WideString& operator+=(wchar_t ch);
  const CFX_WideString& operator+=(const CFX_WideString& str);
  const CFX_WideString& operator+=(const CFX_WideStringC& str);

  bool operator==(const wchar_t* ptr) const;
  bool operator==(const CFX_WideStringC& str) const;
  bool operator==(const CFX_WideString& other) const;

  bool operator!=(const wchar_t* ptr) const { return !(*this == ptr); }
  bool operator!=(const CFX_WideStringC& str) const { return !(*this == str); }
  bool operator!=(const CFX_WideString& other) const {
    return !(*this == other);
  }

  bool operator<(const CFX_WideString& str) const;

  CharType operator[](const size_t index) const {
    ASSERT(IsValidIndex(index));
    return m_pData ? m_pData->m_String[index] : 0;
  }

  CharType First() const { return GetLength() ? (*this)[0] : 0; }
  CharType Last() const { return GetLength() ? (*this)[GetLength() - 1] : 0; }

  void SetAt(size_t index, wchar_t c);

  int Compare(const wchar_t* str) const;
  int Compare(const CFX_WideString& str) const;
  int CompareNoCase(const wchar_t* str) const;

  CFX_WideString Mid(size_t first, size_t count) const;
  CFX_WideString Left(size_t count) const;
  CFX_WideString Right(size_t count) const;

  size_t Insert(size_t index, wchar_t ch);
  size_t InsertAtFront(wchar_t ch) { return Insert(0, ch); }
  size_t InsertAtBack(wchar_t ch) { return Insert(GetLength(), ch); }
  size_t Delete(size_t index, size_t count = 1);

  void Format(const wchar_t* lpszFormat, ...);
  void FormatV(const wchar_t* lpszFormat, va_list argList);

  void MakeLower();
  void MakeUpper();

  void TrimRight();
  void TrimRight(wchar_t chTarget);
  void TrimRight(const CFX_WideStringC& pTargets);

  void TrimLeft();
  void TrimLeft(wchar_t chTarget);
  void TrimLeft(const CFX_WideStringC& pTargets);

  void Reserve(size_t len);
  wchar_t* GetBuffer(size_t len);
  void ReleaseBuffer(size_t len);

  int GetInteger() const;
  float GetFloat() const;

  pdfium::Optional<size_t> Find(const CFX_WideStringC& pSub,
                                size_t start = 0) const;
  pdfium::Optional<size_t> Find(wchar_t ch, size_t start = 0) const;

  bool Contains(const CFX_WideStringC& lpszSub, size_t start = 0) const {
    return Find(lpszSub, start).has_value();
  }

  bool Contains(char ch, size_t start = 0) const {
    return Find(ch, start).has_value();
  }

  size_t Replace(const CFX_WideStringC& pOld, const CFX_WideStringC& pNew);
  size_t Remove(wchar_t ch);

  CFX_ByteString UTF8Encode() const;
  CFX_ByteString UTF16LE_Encode() const;

 protected:
  using StringData = CFX_StringDataTemplate<wchar_t>;

  void ReallocBeforeWrite(size_t nLen);
  void AllocBeforeWrite(size_t nLen);
  void AllocCopy(CFX_WideString& dest,
                 size_t nCopyLen,
                 size_t nCopyIndex) const;
  void AssignCopy(const wchar_t* pSrcData, size_t nSrcLen);
  void Concat(const wchar_t* lpszSrcData, size_t nSrcLen);

  // Returns true unless we ran out of space.
  bool TryVSWPrintf(size_t size, const wchar_t* format, va_list argList);

  CFX_RetainPtr<StringData> m_pData;

  friend class fxcrt_WideStringConcatInPlace_Test;
  friend class fxcrt_WideStringPool_Test;
};

inline CFX_WideString operator+(const CFX_WideStringC& str1,
                                const CFX_WideStringC& str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const CFX_WideStringC& str1,
                                const wchar_t* str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const wchar_t* str1,
                                const CFX_WideStringC& str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const CFX_WideStringC& str1, wchar_t ch) {
  return CFX_WideString(str1, CFX_WideStringC(ch));
}
inline CFX_WideString operator+(wchar_t ch, const CFX_WideStringC& str2) {
  return CFX_WideString(ch, str2);
}
inline CFX_WideString operator+(const CFX_WideString& str1,
                                const CFX_WideString& str2) {
  return CFX_WideString(str1.AsStringC(), str2.AsStringC());
}
inline CFX_WideString operator+(const CFX_WideString& str1, wchar_t ch) {
  return CFX_WideString(str1.AsStringC(), CFX_WideStringC(ch));
}
inline CFX_WideString operator+(wchar_t ch, const CFX_WideString& str2) {
  return CFX_WideString(ch, str2.AsStringC());
}
inline CFX_WideString operator+(const CFX_WideString& str1,
                                const wchar_t* str2) {
  return CFX_WideString(str1.AsStringC(), str2);
}
inline CFX_WideString operator+(const wchar_t* str1,
                                const CFX_WideString& str2) {
  return CFX_WideString(str1, str2.AsStringC());
}
inline CFX_WideString operator+(const CFX_WideString& str1,
                                const CFX_WideStringC& str2) {
  return CFX_WideString(str1.AsStringC(), str2);
}
inline CFX_WideString operator+(const CFX_WideStringC& str1,
                                const CFX_WideString& str2) {
  return CFX_WideString(str1, str2.AsStringC());
}
inline bool operator==(const wchar_t* lhs, const CFX_WideString& rhs) {
  return rhs == lhs;
}
inline bool operator==(const CFX_WideStringC& lhs, const CFX_WideString& rhs) {
  return rhs == lhs;
}
inline bool operator!=(const wchar_t* lhs, const CFX_WideString& rhs) {
  return rhs != lhs;
}
inline bool operator!=(const CFX_WideStringC& lhs, const CFX_WideString& rhs) {
  return rhs != lhs;
}

uint32_t FX_HashCode_GetW(const CFX_WideStringC& str, bool bIgnoreCase);

std::wostream& operator<<(std::wostream& os, const CFX_WideString& str);
std::ostream& operator<<(std::ostream& os, const CFX_WideString& str);
std::wostream& operator<<(std::wostream& os, const CFX_WideStringC& str);
std::ostream& operator<<(std::ostream& os, const CFX_WideStringC& str);

namespace std {

template <>
struct hash<CFX_WideString> {
  std::size_t operator()(const CFX_WideString& str) const {
    return FX_HashCode_GetW(str.AsStringC(), false);
  }
};

}  // namespace std

extern template struct std::hash<CFX_WideString>;

#endif  // CORE_FXCRT_CFX_WIDESTRING_H_
