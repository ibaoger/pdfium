// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_FILEACCESS_POSIX_H_
#define CORE_FXCRT_CFX_FILEACCESS_POSIX_H_

#include "core/fxcrt/ifx_fileaccess.h"

#if _FX_PLATFORM_ == _FX_PLATFORM_LINUX_ || \
    _FX_PLATFORM_ == _FX_PLATFORM_APPLE_ || \
    _FX_PLATFORM_ == _FX_PLATFORM_ANDROID_
class CFX_FileAccess_Posix : public IFX_FileAccess {
 public:
  CFX_FileAccess_Posix();
  ~CFX_FileAccess_Posix() override;

  // IFX_FileAccess:
  bool Open(const ByteStringView& fileName, uint32_t dwMode) override;
  bool Open(const WideStringView& fileName, uint32_t dwMode) override;
  void Close() override;
  FX_FILESIZE GetSize() const override;
  FX_FILESIZE GetPosition() const override;
  FX_FILESIZE SetPosition(FX_FILESIZE pos) override;
  size_t Read(void* pBuffer, size_t szBuffer) override;
  size_t Write(const void* pBuffer, size_t szBuffer) override;
  size_t ReadPos(void* pBuffer, size_t szBuffer, FX_FILESIZE pos) override;
  size_t WritePos(const void* pBuffer,
                  size_t szBuffer,
                  FX_FILESIZE pos) override;
  bool Flush() override;
  bool Truncate(FX_FILESIZE szFile) override;

 protected:
  int32_t m_nFD;
};
#endif

#endif  // CORE_FXCRT_CFX_FILEACCESS_POSIX_H_
