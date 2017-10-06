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
  class PageInfo {
   public:
    PageInfo();
    ~PageInfo();
    PageInfo(const PageInfo& other);
    PageInfo(PageInfo&& other);

    void set_objects_count(uint32_t objects_count) {
      m_nObjectsCount = objects_count;
    }
    uint32_t objects_count() const { return m_nObjectsCount; }

    void set_page_offset(FX_FILESIZE offset) { m_szOffset = offset; }
    uint32_t page_offset() const { return m_szOffset; }

    void set_page_length(uint32_t length) { m_dwLength = length; }
    uint32_t page_length() const { return m_dwLength; }

    void set_start_obj_num(uint32_t start_obj_num) {
      m_dwStartObjNum = start_obj_num;
    }
    uint32_t start_obj_num() const { return m_dwStartObjNum; }

    void AddIdentifier(uint32_t Identifier) {
      m_dwIdentifierArray.push_back(Identifier);
    }

    const std::vector<uint32_t>& Identifiers() { return m_dwIdentifierArray; }

   private:
    uint32_t m_nObjectsCount = 0;
    FX_FILESIZE m_szOffset = 0;
    uint32_t m_dwLength = 0;
    uint32_t m_dwStartObjNum = 0;
    std::vector<uint32_t> m_dwIdentifierArray;
  };

  CPDF_HintTables(CPDF_ReadValidator* pValidator,
                  CPDF_LinearizedHeader* pLinearized);
  virtual ~CPDF_HintTables();

  bool GetPagePos(uint32_t index,
                  FX_FILESIZE* szPageStartPos,
                  FX_FILESIZE* szPageLength,
                  uint32_t* dwObjNum) const;

  CPDF_DataAvail::DocAvailStatus CheckPage(uint32_t index);

  bool LoadHintStream(CPDF_Stream* pHintStream);

  const std::vector<PageInfo>& PageInfos() const { return m_PageInfos; }

 protected:
  bool ReadPageHintTable(CFX_BitStream* hStream);
  bool ReadSharedObjHintTable(CFX_BitStream* hStream, uint32_t offset);

 private:
  uint32_t GetItemLength(uint32_t index,
                         const std::vector<FX_FILESIZE>& szArray) const;

  FX_FILESIZE HintsOffsetToFileOffset(uint32_t hints_offset) const;

  // Owned by |m_pDataAvail|.
  UnownedPtr<CPDF_ReadValidator> m_pValidator;

  // Owned by |m_pDataAvail|.
  UnownedPtr<CPDF_LinearizedHeader> const m_pLinearized;

  uint32_t m_nFirstPageSharedObjs;
  FX_FILESIZE m_szFirstPageObjOffset;

  std::vector<PageInfo> m_PageInfos;
  std::vector<uint32_t> m_dwSharedObjNumArray;
  std::vector<FX_FILESIZE> m_szSharedObjOffsetArray;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_HINT_TABLES_H_
