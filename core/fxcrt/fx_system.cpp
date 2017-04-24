// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_system.h"

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

namespace {

void IgnoreHandler(const wchar_t* expression,
                   const wchar_t* function,
                   const wchar_t* file,
                   unsigned int line,
                   uintptr_t pReserved) {}

}  // namespace

size_t FXSYS_wcsftime(wchar_t* strDest,
                      size_t maxsize,
                      const wchar_t* format,
                      const struct tm* timeptr) {
  auto old_handler = _set_thread_local_invalid_parameter_handler(IgnoreHandler);
  size_t ret = wcsftime(strDest, maxsize, format, timeptr);
  _set_thread_local_invalid_parameter_handler(old_hander);
  return ret;
}

#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
