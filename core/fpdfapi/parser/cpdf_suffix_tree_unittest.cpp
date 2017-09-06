// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_suffix_tree.h"

#include <list>
#include <string>

#include "testing/gtest/include/gtest/gtest.h"

TEST(CPDF_SuffixTreeTest, SearchOneSubstring) {
  CPDF_SuffixTree suffixTree;
  const CFX_ByteString substring("dddd");
  suffixTree.AddString(substring, &substring);
  const std::string data("aaa bbbbbb cccc ddddd eeeee ffff");
  for (const auto& ch : data) {
    if (suffixTree.CheckChar(ch))
      break;
  }
  ASSERT_TRUE(suffixTree.GetResult());
  const CPDF_SuffixTree::Result* result = suffixTree.GetResult();
  EXPECT_EQ(16ul, result->start());
  EXPECT_EQ(20ul, result->end());
  EXPECT_EQ(substring, result->string());
  EXPECT_EQ(&substring, result->user_data());
}

TEST(CPDF_SuffixTreeTest, SearchTwoSubstrings) {
  CPDF_SuffixTree suffixTree;
  const CFX_ByteString substring1("ccc");
  const CFX_ByteString substring2("dddd");
  suffixTree.AddString(substring1, &substring1);
  suffixTree.AddString(substring2, &substring2);
  const std::string data("aaa bbbbbb cccc ddddd eeeee ffff");
  for (const auto& ch : data) {
    if (suffixTree.CheckChar(ch))
      break;
  }
  ASSERT_TRUE(suffixTree.GetResult());
  const CPDF_SuffixTree::Result* result = suffixTree.GetResult();
  EXPECT_EQ(11ul, result->start());
  EXPECT_EQ(14ul, result->end());
  EXPECT_EQ(substring1, result->string());
  EXPECT_EQ(&substring1, result->user_data());
}

TEST(CPDF_SuffixTreeTest, SearchTwoResemblingSubstrings) {
  CPDF_SuffixTree suffixTree;
  const CFX_ByteString substring1("eeeee");
  const CFX_ByteString substring2("eee");
  suffixTree.AddString(substring1, &substring1);
  suffixTree.AddString(substring2, &substring2);
  const std::string data("aaa bbbbbb cccc ddddd eeeee ffff");
  for (const auto& ch : data) {
    if (suffixTree.CheckChar(ch))
      break;
  }
  ASSERT_TRUE(suffixTree.GetResult());
  const CPDF_SuffixTree::Result* result = suffixTree.GetResult();
  EXPECT_EQ(22ul, result->start());
  EXPECT_EQ(27ul, result->end());
  EXPECT_EQ(substring1, result->string());
  EXPECT_EQ(&substring1, result->user_data());
}

TEST(CPDF_SuffixTreeTest, SearchNotExistsSubstring) {
  CPDF_SuffixTree suffixTree;
  const CFX_ByteString substring("www");
  suffixTree.AddString(substring, &substring);
  const std::string data("aaa bbbbbb cccc ddddd eeeee ffff");
  for (const auto& ch : data) {
    if (suffixTree.CheckChar(ch))
      break;
  }
  EXPECT_FALSE(suffixTree.GetResult());
}

TEST(CPDF_SuffixTreeTest, SearchBackOrderedSubstring) {
  CPDF_SuffixTree suffixTree;
  const CFX_ByteString substring("trewq");
  suffixTree.AddStringInBackOrder(substring, &substring);
  const std::string data("aaa bbbbbb cccc qwert eeeee ffff");
  for (const auto& ch : data) {
    if (suffixTree.CheckChar(ch))
      break;
  }
  ASSERT_TRUE(suffixTree.GetResult());
  const CPDF_SuffixTree::Result* result = suffixTree.GetResult();
  EXPECT_EQ(16ul, result->start());
  EXPECT_EQ(21ul, result->end());
  EXPECT_EQ(substring, result->string());
  EXPECT_EQ(&substring, result->user_data());
}

TEST(CPDF_SuffixTreeTest, SearchWholeWord) {
  CPDF_SuffixTree suffixTree;
  suffixTree.SetMode(CPDF_SuffixTree::Mode::kWholeWord);
  const CFX_ByteString substring("word");
  suffixTree.AddString(substring, &substring);
  const std::string data("aaa notword(word/qwert eeeee ffff");
  for (const auto& ch : data) {
    if (suffixTree.CheckChar(ch))
      break;
  }
  ASSERT_TRUE(suffixTree.GetResult());
  const CPDF_SuffixTree::Result* result = suffixTree.GetResult();
  EXPECT_EQ(12ul, result->start());
  EXPECT_EQ(16ul, result->end());
  EXPECT_EQ(substring, result->string());
  EXPECT_EQ(&substring, result->user_data());
}

TEST(CPDF_SuffixTreeTest, SearchWholeKeyword) {
  CPDF_SuffixTree suffixTree;
  suffixTree.SetMode(CPDF_SuffixTree::Mode::kWholeKeyword);
  const CFX_ByteString substring("keyword");
  suffixTree.AddString(substring, &substring);
  const std::string data("aaa notkeyword not(keyword/qwert keyword eeeee");
  for (const auto& ch : data) {
    if (suffixTree.CheckChar(ch))
      break;
  }
  ASSERT_TRUE(suffixTree.GetResult());
  const CPDF_SuffixTree::Result* result = suffixTree.GetResult();
  EXPECT_EQ(33ul, result->start());
  EXPECT_EQ(40ul, result->end());
  EXPECT_EQ(substring, result->string());
  EXPECT_EQ(&substring, result->user_data());
}

TEST(CPDF_SuffixTreeTest, SearchWholeKeywordWithSpaces) {
  CPDF_SuffixTree suffixTree;
  suffixTree.SetMode(CPDF_SuffixTree::Mode::kWholeKeyword);
  const CFX_ByteString substring("\r\nkeyword ");
  suffixTree.AddString(substring, &substring);
  const std::string data("aaa notkeyword not(keyword/qwert\r\nkeyword eeeee");
  for (const auto& ch : data) {
    if (suffixTree.CheckChar(ch))
      break;
  }
  ASSERT_TRUE(suffixTree.GetResult());
  const CPDF_SuffixTree::Result* result = suffixTree.GetResult();
  EXPECT_EQ(32ul, result->start());
  EXPECT_EQ(42ul, result->end());
  EXPECT_EQ(substring, result->string());
  EXPECT_EQ(&substring, result->user_data());
}

TEST(CPDF_SuffixTreeTest, SearchAtAnd) {
  CPDF_SuffixTree suffixTree;
  const CFX_ByteString substring("end");
  suffixTree.AddString(substring, &substring);
  const std::string data("aaa notkeyword not(keyword/qwert keyword end");
  for (const auto& ch : data) {
    EXPECT_FALSE(suffixTree.CheckChar(ch));
  }
  EXPECT_TRUE(suffixTree.CheckEOF());
  ASSERT_TRUE(suffixTree.GetResult());
  const CPDF_SuffixTree::Result* result = suffixTree.GetResult();
  EXPECT_EQ(41ul, result->start());
  EXPECT_EQ(44ul, result->end());
  EXPECT_EQ(data.size(), result->end());
  EXPECT_EQ(substring, result->string());
  EXPECT_EQ(&substring, result->user_data());
}
