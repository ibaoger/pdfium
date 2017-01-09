// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fde/cfde_txtedtbuf.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"
#include "third_party/base/ptr_util.h"

class CFDE_TxtEdtBufTest : public testing::Test {
 public:
  void SetUp() override {
    buf = pdfium::MakeUnique<CFDE_TxtEdtBuf>();
    buf->SetChunkSizeForTesting(5);
  }
  size_t ChunkCount() const { return buf->m_chunks.size(); }

  std::unique_ptr<CFDE_TxtEdtBuf> buf;
};

typedef CFDE_TxtEdtBufTest CFDE_TxtEdtBufTestDeathTest;

TEST_F(CFDE_TxtEdtBufTest, SetTextLessThenChunkSize) {
  buf->SetText(L"Hi");
  EXPECT_EQ(1UL, ChunkCount());
  EXPECT_EQ(L"Hi", buf->GetText());
  EXPECT_EQ(2, buf->GetTextLength());
}

TEST_F(CFDE_TxtEdtBufTest, InsertAppendChunk) {
  buf->SetText(L"Hi");
  EXPECT_EQ(1UL, ChunkCount());

  CFX_WideString end = L" World";
  buf->Insert(2, end.c_str(), end.GetLength());
  EXPECT_EQ(3UL, ChunkCount());
  EXPECT_EQ(L"Hi World", buf->GetText());
  EXPECT_EQ(8, buf->GetTextLength());
}

TEST_F(CFDE_TxtEdtBufTest, InsertPrependChunk) {
  buf->SetText(L"Hi");
  EXPECT_EQ(1UL, ChunkCount());

  CFX_WideString end = L"World ";
  buf->Insert(0, end.c_str(), end.GetLength());
  EXPECT_EQ(3UL, ChunkCount());
  EXPECT_EQ(L"World Hi", buf->GetText());
  EXPECT_EQ(8, buf->GetTextLength());
}

TEST_F(CFDE_TxtEdtBufTest, InsertBetweenChunks) {
  buf->SetText(L"Hello World");
  EXPECT_EQ(3UL, ChunkCount());

  CFX_WideString inst = L"there ";
  buf->Insert(6, inst.c_str(), inst.GetLength());
  EXPECT_EQ(5UL, ChunkCount());
  EXPECT_EQ(L"Hello there World", buf->GetText());
  EXPECT_EQ(17, buf->GetTextLength());
}

TEST_F(CFDE_TxtEdtBufTestDeathTest, InsertBadIndexes) {
  CFX_WideString inst = L"there ";

  buf->SetText(L"Hi");
  EXPECT_DEATH(buf->Insert(-4, inst.c_str(), inst.GetLength()), "Assertion");
  EXPECT_DEATH(buf->Insert(9999, inst.c_str(), inst.GetLength()), "Assertion");
  EXPECT_DEATH(buf->Insert(1, inst.c_str(), -6), "Assertion");
}

TEST_F(CFDE_TxtEdtBufTest, SetText) {
  buf->SetText(L"Hello World");
  EXPECT_EQ(3UL, ChunkCount());
  EXPECT_EQ(11, buf->GetTextLength());

  buf->SetText(L"Hi");
  // Don't remove chunks on setting shorter text.
  EXPECT_EQ(3UL, ChunkCount());
  EXPECT_EQ(L"Hi", buf->GetText());
  EXPECT_EQ(2, buf->GetTextLength());
}

TEST_F(CFDE_TxtEdtBufTest, DeleteMiddleText) {
  buf->SetText(L"Hello there World");
  buf->Delete(6, 6);
  EXPECT_EQ(4UL, ChunkCount());
  EXPECT_EQ(L"Hello World", buf->GetText());
  EXPECT_EQ(11, buf->GetTextLength());
}

TEST_F(CFDE_TxtEdtBufTest, DeleteEndText) {
  buf->SetText(L"Hello World");
  buf->Delete(5, 6);
  EXPECT_EQ(1UL, ChunkCount());
  EXPECT_EQ(L"Hello", buf->GetText());
  EXPECT_EQ(5, buf->GetTextLength());
}

TEST_F(CFDE_TxtEdtBufTest, DeleteStartText) {
  buf->SetText(L"Hello World");
  buf->Delete(0, 6);
  EXPECT_EQ(2UL, ChunkCount());
  EXPECT_EQ(L"World", buf->GetText());
  EXPECT_EQ(5, buf->GetTextLength());
}

TEST_F(CFDE_TxtEdtBufTest, DeleteAllText) {
  buf->SetText(L"Hello World");
  buf->Delete(0, 11);
  EXPECT_EQ(0UL, ChunkCount());
  EXPECT_EQ(L"", buf->GetText());
  EXPECT_EQ(0, buf->GetTextLength());
}

TEST_F(CFDE_TxtEdtBufTestDeathTest, DeleteWithBadIdx) {
  buf->SetText(L"Hi");
  EXPECT_DEATH(buf->Delete(-10, 4), "Assertion");
  EXPECT_DEATH(buf->Delete(1, -5), "Assertion");
  EXPECT_DEATH(buf->Delete(5, 1), "Assertion");
  EXPECT_DEATH(buf->Delete(0, 10000), "Assertion");
}

TEST_F(CFDE_TxtEdtBufTest, ClearWithRelease) {
  buf->SetText(L"Hello World");
  buf->Clear(true);
  EXPECT_EQ(0UL, ChunkCount());
  EXPECT_EQ(L"", buf->GetText());
  EXPECT_EQ(0, buf->GetTextLength());
}

TEST_F(CFDE_TxtEdtBufTest, ClearWithoutRelease) {
  buf->SetText(L"Hello World");
  buf->Clear(false);
  EXPECT_EQ(3UL, ChunkCount());
  EXPECT_EQ(L"", buf->GetText());
  EXPECT_EQ(0, buf->GetTextLength());
}

TEST_F(CFDE_TxtEdtBufTest, GetCharByIndex) {
  buf->SetText(L"Hello world");
  EXPECT_EQ(L"e", CFX_WideString(buf->GetCharByIndex(1)));
  EXPECT_EQ(L"o", CFX_WideString(buf->GetCharByIndex(7)));
}

TEST_F(CFDE_TxtEdtBufTestDeathTest, GetCharByIndex) {
  buf->SetText(L"Hi");
  EXPECT_DEATH(buf->GetCharByIndex(-1), "Assertion");
  EXPECT_DEATH(buf->GetCharByIndex(100), "Assertion");
}

TEST_F(CFDE_TxtEdtBufTest, GetRange) {
  buf->SetText(L"Hello World");
  EXPECT_EQ(L"", buf->GetRange(1, 0));
  EXPECT_EQ(L"ello", buf->GetRange(1, 4));
  EXPECT_EQ(L"lo Wo", buf->GetRange(3, 5));
}

TEST_F(CFDE_TxtEdtBufTestDeathTest, GetRange) {
  buf->SetText(L"Hi");
  EXPECT_DEATH(buf->GetRange(1, -1), "Assertion");
  EXPECT_DEATH(buf->GetRange(-1, 1), "Assertion");
  EXPECT_DEATH(buf->GetRange(10, 1), "Assertion");
  EXPECT_DEATH(buf->GetRange(1, 100), "Assertion");
}
