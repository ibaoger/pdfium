// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_EDIT_CPDF_FLATEENCODER_H_
#define CORE_FPDFAPI_EDIT_CPDF_FLATEENCODER_H_

#include <memory>
#include <utility>
#include <vector>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcrt/cfx_maybe_owned.h"
#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_memory.h"

class CPDF_Stream;

class CPDF_FlateEncoder {
 public:
  CPDF_FlateEncoder(const CPDF_Stream* pStream, bool bFlateEncode);
  ~CPDF_FlateEncoder();

  uint32_t GetSize() const { return m_dwSize; }
  uint8_t* GetData() const { return m_pData.Get(); }

  std::vector<std::pair<const CFX_ByteStringC, std::unique_ptr<CPDF_Object>>>&
  changed_values() {
    return m_ChangedValues;
  }

 private:
  uint32_t m_dwSize;
  CFX_MaybeOwned<uint8_t, FxFreeDeleter> m_pData;
  CFX_RetainPtr<CPDF_StreamAcc> m_pAcc;
  std::vector<std::pair<const CFX_ByteStringC, std::unique_ptr<CPDF_Object>>>
      m_ChangedValues;
  std::vector<std::unique_ptr<CPDF_Object>> m_ObjectsStore;
};

#endif  // CORE_FPDFAPI_EDIT_CPDF_FLATEENCODER_H_
