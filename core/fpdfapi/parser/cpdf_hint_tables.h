// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_HINT_TABLES_H_
#define CORE_FPDFAPI_PARSER_CPDF_HINT_TABLES_H_

#include <vector>

#include "core/fpdfapi/parser/cpdf_data_avail.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/unowned_ptr.h"

class CFX_BitStream;
class CPDF_LinearizedHeader;
class CPDF_Stream;
class CPDF_ReadValidator;

class CPDF_HintTables {
 public:
  CPDF_HintTables(CPDF_ReadValidator* pValidator,
                  CPDF_LinearizedHeader* pLinearized);
  virtual ~CPDF_HintTables();

  bool GetPagePos(uint32_t index,
                  FX_FILESIZE* szPageStartPos,
                  FX_FILESIZE* szPageLength,
                  uint32_t* dwObjNum);

  CPDF_DataAvail::DocAvailStatus CheckPage(uint32_t index);

  bool LoadHintStream(CPDF_Stream* pHintStream);

 protected:
  bool ReadPageHintTable(CFX_BitStream* hStream);
  bool ReadSharedObjHintTable(CFX_BitStream* hStream, uint32_t offset);

 private:
  uint32_t GetItemLength(uint32_t index,
                         const std::vector<FX_FILESIZE>& szArray);

  // Owned by |m_pDataAvail|.
  UnownedPtr<CPDF_ReadValidator> m_pValidator;

  // Owned by |m_pDataAvail|.
  UnownedPtr<CPDF_LinearizedHeader> const m_pLinearized;

  uint32_t m_nFirstPageSharedObjs;
  FX_FILESIZE m_szFirstPageObjOffset;
  std::vector<uint32_t> m_dwDeltaNObjsArray;
  std::vector<uint32_t> m_dwNSharedObjsArray;
  std::vector<uint32_t> m_dwSharedObjNumArray;
  std::vector<uint32_t> m_dwIdentifierArray;
  std::vector<FX_FILESIZE> m_szPageOffsetArray;
  std::vector<FX_FILESIZE> m_szSharedObjOffsetArray;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_HINT_TABLES_H_
