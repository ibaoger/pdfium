// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_extension.h"

#include <algorithm>
#include <limits>

#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/utf16.h"

namespace {

time_t DefaultTimeFunction() {
  return time(nullptr);
}

struct tm* DefaultLocaltimeFunction(const time_t* tp) {
  return localtime(tp);
}

time_t (*g_time_func)() = DefaultTimeFunction;
struct tm* (*g_localtime_func)(const time_t*) = DefaultLocaltimeFunction;

}  // namespace

float FXSYS_wcstof(const wchar_t* pwsStr, size_t nLength, size_t* pUsedLen) {
  // SAFETY: TODO(tsepez): This is an enormous unsafe block, pretty hard to
  // explain its soundness.
  UNSAFE_BUFFERS({
    DCHECK(pwsStr);
    if (nLength == 0) {
      return 0.0f;
    }

    size_t nUsedLen = 0;
    bool bNegtive = false;
    switch (pwsStr[nUsedLen]) {
      case '-':
        bNegtive = true;
        [[fallthrough]];
      case '+':
        nUsedLen++;
        break;
    }

    double dValue = 0.0f;
    while (nUsedLen < nLength) {
      wchar_t wch = pwsStr[nUsedLen];
      if (!FXSYS_IsDecimalDigit(wch))
        break;

      dValue = dValue * 10.0f + (wch - L'0');
      nUsedLen++;
    }

    if (nUsedLen < nLength && pwsStr[nUsedLen] == L'.') {
      float fPrecise = 0.1f;
      while (++nUsedLen < nLength) {
        wchar_t wch = pwsStr[nUsedLen];
        if (!FXSYS_IsDecimalDigit(wch)) {
          break;
        }

        dValue += (wch - L'0') * fPrecise;
        fPrecise *= 0.1f;
      }
    }

    if (nUsedLen < nLength &&
        (pwsStr[nUsedLen] == 'e' || pwsStr[nUsedLen] == 'E')) {
      ++nUsedLen;

      bool negative_exponent = false;
      if (nUsedLen < nLength &&
          (pwsStr[nUsedLen] == '-' || pwsStr[nUsedLen] == '+')) {
        negative_exponent = pwsStr[nUsedLen] == '-';
        ++nUsedLen;
      }

      int32_t exp_value = 0;
      while (nUsedLen < nLength) {
        wchar_t wch = pwsStr[nUsedLen];
        if (!FXSYS_IsDecimalDigit(wch)) {
          break;
        }

        exp_value = exp_value * 10.0f + (wch - L'0');
        // Exponent is outside the valid range, fail.
        if ((negative_exponent &&
             -exp_value < std::numeric_limits<float>::min_exponent10) ||
            (!negative_exponent &&
             exp_value > std::numeric_limits<float>::max_exponent10)) {
          if (pUsedLen) {
            *pUsedLen = 0;
          }
          return 0.0f;
        }

        ++nUsedLen;
      }

      for (size_t i = exp_value; i > 0; --i) {
        if (exp_value > 0) {
          if (negative_exponent) {
            dValue /= 10;
          } else {
            dValue *= 10;
          }
        }
      }
    }

    if (pUsedLen) {
      *pUsedLen = nUsedLen;
    }

    return static_cast<float>(bNegtive ? -dValue : dValue);
  });
}

wchar_t* FXSYS_wcsncpy(wchar_t* dstStr, const wchar_t* srcStr, size_t count) {
  DCHECK(dstStr);
  DCHECK(srcStr);
  DCHECK(count > 0);

  // SAFETY: TODO(tsepez): This is UNSAFE_BUFFER_USAGE as well.
  UNSAFE_BUFFERS({
    for (size_t i = 0; i < count; ++i) {
      dstStr[i] = srcStr[i];
      if (dstStr[i] == L'\0') {
        break;
      }
    }
  });
  return dstStr;
}

void FXSYS_IntToTwoHexChars(uint8_t n, char* buf) {
  static const char kHex[] = "0123456789ABCDEF";
  // SAFETY: TODO(tsepez): This is UNSAFE_BUFFER_USAGE as well.
  UNSAFE_BUFFERS({
    buf[0] = kHex[n / 16];
    buf[1] = kHex[n % 16];
  });
}

void FXSYS_IntToFourHexChars(uint16_t n, char* buf) {
  // SAFETY: TODO(tsepez): This is UNSAFE_BUFFER_USAGE as well.
  UNSAFE_BUFFERS({
    FXSYS_IntToTwoHexChars(n / 256, buf);
    FXSYS_IntToTwoHexChars(n % 256, buf + 2);
  });
}

size_t FXSYS_ToUTF16BE(uint32_t unicode, char* buf) {
  DCHECK(unicode <= pdfium::kMaximumSupplementaryCodePoint);
  DCHECK(!pdfium::IsHighSurrogate(unicode));
  DCHECK(!pdfium::IsLowSurrogate(unicode));

  if (unicode <= 0xFFFF) {
    FXSYS_IntToFourHexChars(unicode, buf);
    return 4;
  }
  // SAFETY: TODO(tsepez): This is UNSAFE_BUFFER_USAGE as well.
  UNSAFE_BUFFERS({
    pdfium::SurrogatePair surrogate_pair(unicode);
    FXSYS_IntToFourHexChars(surrogate_pair.high(), buf);
    FXSYS_IntToFourHexChars(surrogate_pair.low(), buf + 4);
  });
  return 8;
}

void FXSYS_SetTimeFunction(time_t (*func)()) {
  g_time_func = func ? func : DefaultTimeFunction;
}

void FXSYS_SetLocaltimeFunction(struct tm* (*func)(const time_t*)) {
  g_localtime_func = func ? func : DefaultLocaltimeFunction;
}

time_t FXSYS_time(time_t* tloc) {
  time_t ret_val = g_time_func();
  if (tloc)
    *tloc = ret_val;
  return ret_val;
}

struct tm* FXSYS_localtime(const time_t* tp) {
  return g_localtime_func(tp);
}
