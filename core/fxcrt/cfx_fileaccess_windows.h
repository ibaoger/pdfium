// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_FILEACCESS_WINDOWS_H_
#define CORE_FXCRT_CFX_FILEACCESS_WINDOWS_H_

#include <stddef.h>
#include <stdint.h>

#include "build/build_config.h"
#include "core/fxcrt/fileaccess_iface.h"
#include "core/fxcrt/fx_types.h"

#if !BUILDFLAG(IS_WIN)
#error "Included on the wrong platform"
#endif

class CFX_FileAccess_Windows final : public FileAccessIface {
 public:
  CFX_FileAccess_Windows();
  ~CFX_FileAccess_Windows() override;

  // FileAccessIface
  bool Open(ByteStringView fileName, uint32_t dwMode) override;
  bool Open(WideStringView fileName, uint32_t dwMode) override;
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

 private:
  void* m_hFile = nullptr;
};

#endif  // CORE_FXCRT_CFX_FILEACCESS_WINDOWS_H_
