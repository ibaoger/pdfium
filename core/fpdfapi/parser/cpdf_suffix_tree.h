// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FPDFAPI_PARSER_CPDF_SUFFIX_TREE_H_
#define CORE_FPDFAPI_PARSER_CPDF_SUFFIX_TREE_H_

#include <list>
#include <memory>
#include <string>

#include "core/fxcrt/fx_string.h"

class CPDF_SuffixTree {
 public:
  enum class Mode {
    kAny,
    kWholeWord,
    kWholeKeyword,
  };

  class Result {
   public:
    Result(uint32_t dwStart,
           uint32_t dwEnd,
           const CFX_ByteString& string,
           const void* pUserData);

    uint32_t end() const { return m_dwEnd; }
    uint32_t start() const { return m_dwStart; }
    const CFX_ByteString& string() const { return m_string; }
    const void* user_data() const { return m_pUserData; }

   private:
    const uint32_t m_dwStart;
    const uint32_t m_dwEnd;
    const CFX_ByteString m_string;
    const void* m_pUserData;
  };

  CPDF_SuffixTree();
  ~CPDF_SuffixTree();

  void SetMode(Mode mode) { m_Mode = mode; }
  Mode mode() const { return m_Mode; }

  void AddString(const CFX_ByteString& str, const void* pUserData);
  void AddStringInBackOrder(const CFX_ByteString& str, const void* pUserData);

  bool CheckChar(uint8_t ch);
  bool CheckEOF();
  void Restart();
  void Reset();

  const Result* GetResult() const { return m_pResult.get(); }
  uint32_t GetProcessedCharsCount() const { return m_NextIndex; }

 private:
  void CheckCandidates(uint8_t ch);
  bool IsValidDelimiterChar(uint8_t ch) const;

  bool HasResultCandidates() const { return !m_ResultCandidates.empty(); }
  bool HasResults() const { return !m_Results.empty(); }
  bool SetFinished();

 private:
  class Node;
  class Item;
  std::unique_ptr<Node> m_pRoot;
  bool m_bFinished = false;
  std::list<Result> m_ResultCandidates;
  std::list<Result> m_Results;
  std::unique_ptr<Result> m_pResult;
  std::list<Item> m_CurrentItems;
  Mode m_Mode = Mode::kAny;
  uint8_t m_cPrevChar = 0;
  uint32_t m_NextIndex = 0;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_SUFFIX_TREE_H_
