// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_system.h"
#include "xfa/fgas/localization/fgas_datetime.h"

#if _FX_OS_ == _FX_LINUX_DESKTOP_ || _FX_OS_ == _FX_ANDROID_ || \
    _FX_OS_ == _FX_MACOSX_ || _FX_OS_ == _FX_IOS_
#include <sys/time.h>
#include <time.h>
#endif

namespace {

const uint8_t g_FXDaysPerMonth[12] = {31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31};
const uint8_t g_FXDaysPerLeapMonth[12] = {31, 29, 31, 30, 31, 30,
                                          31, 31, 30, 31, 30, 31};
const int32_t g_FXDaysBeforeMonth[12] = {0,   31,  59,  90,  120, 151,
                                         181, 212, 243, 273, 304, 334};
const int32_t g_FXDaysBeforeLeapMonth[12] = {0,   31,  60,  91,  121, 152,
                                             182, 213, 244, 274, 305, 335};
const int32_t g_FXDaysPerYear = 365;
const int32_t g_FXDaysPerLeapYear = 366;

int32_t DaysBeforeMonthInYear(int32_t iYear, uint8_t iMonth) {
  ASSERT(iYear != 0);
  ASSERT(iMonth >= 1 && iMonth <= 12);

  const int32_t* p =
      FX_IsLeapYear(iYear) ? g_FXDaysBeforeLeapMonth : g_FXDaysBeforeMonth;
  return p[iMonth - 1];
}

int32_t DaysInYear(int32_t iYear) {
  ASSERT(iYear != 0);
  return FX_IsLeapYear(iYear) ? g_FXDaysPerLeapYear : g_FXDaysPerYear;
}

int64_t DateToDays(int32_t iYear,
                   uint8_t iMonth,
                   uint8_t iDay,
                   bool bIncludeThisDay) {
  ASSERT(iYear != 0);
  ASSERT(iMonth >= 1 && iMonth <= 12);
  ASSERT(iDay >= 1 && iDay <= FX_DaysInMonth(iYear, iMonth));

  int64_t iDays = DaysBeforeMonthInYear(iYear, iMonth);
  iDays += iDay;
  if (!bIncludeThisDay)
    iDays--;

  if (iYear > 0) {
    iYear--;
  } else {
    iDays -= DaysInYear(iYear);
    iYear++;
  }
  return iDays + static_cast<int64_t>(iYear) * 365 + iYear / 4 - iYear / 100 +
         iYear / 400;
}

struct FXUT_SYSTEMTIME {
  uint16_t wYear;
  uint16_t wMonth;
  uint16_t wDayOfWeek;
  uint16_t wDay;
  uint16_t wHour;
  uint16_t wMinute;
  uint16_t wSecond;
  uint16_t wMilliseconds;
};

}  // namespace

uint8_t FX_DaysInMonth(int32_t iYear, uint8_t iMonth) {
  ASSERT(iYear != 0);
  ASSERT(iMonth >= 1 && iMonth <= 12);

  const uint8_t* p =
      FX_IsLeapYear(iYear) ? g_FXDaysPerLeapMonth : g_FXDaysPerMonth;
  return p[iMonth - 1];
}

bool FX_IsLeapYear(int32_t iYear) {
  ASSERT(iYear != 0);
  return ((iYear % 4) == 0 && (iYear % 100) != 0) || (iYear % 400) == 0;
}

void CFX_Unitime::Now() {
  FXUT_SYSTEMTIME utLocal;
#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || \
    _FX_OS_ == _FX_WIN64_
  ::GetLocalTime((LPSYSTEMTIME)&utLocal);
#elif _FX_OS_ != _FX_EMBEDDED_
  timeval curTime;
  gettimeofday(&curTime, nullptr);

  struct tm st;
  localtime_r(&curTime.tv_sec, &st);
  utLocal.wYear = st.tm_year + 1900;
  utLocal.wMonth = st.tm_mon + 1;
  utLocal.wDayOfWeek = st.tm_wday;
  utLocal.wDay = st.tm_mday;
  utLocal.wHour = st.tm_hour;
  utLocal.wMinute = st.tm_min;
  utLocal.wSecond = st.tm_sec;
  utLocal.wMilliseconds = curTime.tv_usec / 1000;
#endif  // _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || \
        // _FX_OS_ == _FX_WIN64_

  year_ = utLocal.wYear;
  month_ = static_cast<uint8_t>(utLocal.wMonth);
  day_ = static_cast<uint8_t>(utLocal.wDay);
  hour_ = static_cast<uint8_t>(utLocal.wHour);
  minute_ = static_cast<uint8_t>(utLocal.wMinute);
  second_ = static_cast<uint8_t>(utLocal.wSecond);
  milliseconds_ = static_cast<uint16_t>(utLocal.wMilliseconds);
}

int32_t CFX_Unitime::GetDayOfWeek() const {
  int32_t v = static_cast<int32_t>(DateToDays(year_, month_, day_, true) % 7);
  if (v < 0)
    v += 7;
  return v;
}
