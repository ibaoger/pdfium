// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_suffix_tree.h"

#include <vector>

#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/ptr_util.h"

CPDF_SuffixTree::Result::Result(uint32_t dwStart,
                                uint32_t dwEnd,
                                const CFX_ByteString& string,
                                const void* pUserData)
    : m_dwStart(dwStart),
      m_dwEnd(dwEnd),
      m_string(string),
      m_pUserData(pUserData) {}

class CPDF_SuffixTree::Node {
 public:
  Node() {}
  ~Node() {}

  Node* AddEmptyChild(uint8_t ch) {
    if (m_Childs.empty())
      m_Childs.resize(256);
    return &(m_Childs[ch]);
  }

  Node* AddChild(uint8_t ch,
                 const CFX_ByteString& string,
                 const void* pUserData) {
    Node* result = AddEmptyChild(ch);
    result->m_pUserData = pUserData;
    result->m_string = string;
    return result;
  }

  const Node* child_at(uint8_t ch) const {
    if (!m_Childs.empty() &&
        (m_Childs[ch].IsResultNode() || !m_Childs[ch].m_Childs.empty()))
      return &m_Childs[ch];
    return nullptr;
  }

  bool IsResultNode() const { return !m_string.IsEmpty(); }

  const void* user_data() const { return m_pUserData; }
  const CFX_ByteString& string() const { return m_string; }

 private:
  std::vector<Node> m_Childs;
  CFX_ByteString m_string;
  const void* m_pUserData = nullptr;
};

class CPDF_SuffixTree::Item {
 public:
  Item(const Node* pNode, uint32_t dwStart)
      : m_pNode(pNode), m_dwStart(dwStart) {}
  operator bool() const { return !!m_pNode; }
  const Node* node() const { return m_pNode; }
  uint32_t start() const { return m_dwStart; }
  const void* user_data() const { return node()->user_data(); }
  const CFX_ByteString& string() const { return node()->string(); }
  bool IsResultNode() const { return node()->IsResultNode(); }

 private:
  const Node* m_pNode;
  uint32_t m_dwStart;
};

CPDF_SuffixTree::CPDF_SuffixTree() {}

CPDF_SuffixTree::~CPDF_SuffixTree() {}

void CPDF_SuffixTree::AddString(const CFX_ByteString& str,
                                const void* pUserData) {
  if (str.IsEmpty())
    return;

  if (!m_pRoot)
    m_pRoot = pdfium::MakeUnique<Node>();

  Node* current = m_pRoot.get();
  for (FX_STRSIZE i = 0; i < str.GetLength() - 1; ++i) {
    current = current->AddEmptyChild(str[i]);
  }
  current->AddChild(str[str.GetLength() - 1], str, pUserData);
}

void CPDF_SuffixTree::AddStringInBackOrder(const CFX_ByteString& str,
                                           const void* pUserData) {
  if (str.IsEmpty())
    return;

  if (!m_pRoot)
    m_pRoot = pdfium::MakeUnique<Node>();

  Node* current = m_pRoot.get();
  for (FX_STRSIZE i = str.GetLength() - 1; i > 0; --i) {
    current = current->AddEmptyChild(str[i]);
  }
  current->AddChild(str[0], str, pUserData);
}

bool CPDF_SuffixTree::CheckChar(uint8_t ch) {
  if (m_bFinished)
    return !!m_pResult;

  {
    FX_SAFE_UINT32 safeNextIndex = m_NextIndex;
    safeNextIndex += 1;
    if (!safeNextIndex.IsValid())
      return false;
  }

  if (!m_pRoot)
    return false;

  if (HasResultCandidates()) {
    CheckCandidates(ch);
  }

  if (!HasResults()) {
    if (IsValidDelimiterChar(m_cPrevChar) || IsValidDelimiterChar(ch)) {
      m_CurrentItems.push_back(Item(m_pRoot.get(), m_NextIndex));
    }
  } else if (m_CurrentItems.empty()) {
    return SetFinished();
  }

  std::list<Item> nextStepItems;
  ++m_NextIndex;
  m_cPrevChar = ch;
  for (const auto& it : m_CurrentItems) {
    Item item(it.node()->child_at(ch), it.start());
    if (item) {
      nextStepItems.push_back(item);
      if (item.IsResultNode()) {
        m_ResultCandidates.push_back(
            Result(item.start(), m_NextIndex, item.string(), item.user_data()));
      }
    }
  }
  nextStepItems.swap(m_CurrentItems);
  return false;
}

bool CPDF_SuffixTree::CheckEOF() {
  if (m_bFinished)
    return !!m_pResult;

  CheckCandidates(0);
  return SetFinished();
}

void CPDF_SuffixTree::Restart() {
  m_bFinished = false;
  m_pResult.reset();
  m_ResultCandidates.clear();
  m_Results.clear();
  m_CurrentItems.clear();
  m_cPrevChar = 0;
  m_NextIndex = 0;
}

void CPDF_SuffixTree::Reset() {
  Restart();
  m_pRoot.reset();
}

void CPDF_SuffixTree::CheckCandidates(uint8_t ch) {
  for (const auto& it : m_ResultCandidates) {
    if (IsValidDelimiterChar(ch) || IsValidDelimiterChar(it.string().Last()))
      m_Results.push_back(it);
  }
  m_ResultCandidates.clear();
}

bool CPDF_SuffixTree::IsValidDelimiterChar(uint8_t ch) const {
  switch (m_Mode) {
    case Mode::kAny:
      return true;
    case Mode::kWholeWord:
      return PDFCharIsDelimiter(ch) || PDFCharIsWhitespace(ch);
    case Mode::kWholeKeyword:
      return PDFCharIsWhitespace(ch);
  }
}

bool CPDF_SuffixTree::SetFinished() {
  m_bFinished = true;
  if (HasResults()) {
    const Result* result = &*(m_Results.begin());
    for (const auto& it : m_Results) {
      if (result->start() > it.start()) {
        result = &it;
      } else if (result->start() == it.start() && result->end() < it.end()) {
        result = &it;
      }
    }
    m_pResult = pdfium::MakeUnique<Result>(*result);
  }
  m_Results.clear();
  m_pRoot.reset();
  m_ResultCandidates.clear();
  m_CurrentItems.clear();
  return !!m_pResult;
}
