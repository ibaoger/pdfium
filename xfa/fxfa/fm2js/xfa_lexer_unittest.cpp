// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/fm2js/xfa_lexer.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"
#include "third_party/base/ptr_util.h"

TEST(CXFA_FMLexerTest, EmptyString) {
  CXFA_FMLexer lexer(L"", nullptr);
  CXFA_FMToken* token = lexer.NextToken();
  EXPECT_EQ(TOKeof, token->m_type);
}

TEST(CXFA_FMLexerTest, Numbers) {
  auto lexer = pdfium::MakeUnique<CXFA_FMLexer>(L"-12", nullptr);
  CXFA_FMToken* token = lexer->NextToken();
  // This should probably return -12 as the number instead of 2 tokens.
  EXPECT_EQ(TOKminus, token->m_type);
  token = lexer->NextToken();
  EXPECT_EQ(L"12", token->m_wstring);

  lexer = pdfium::MakeUnique<CXFA_FMLexer>(L"1.5362", nullptr);
  token = lexer->NextToken();
  EXPECT_EQ(TOKnumber, token->m_type);
  EXPECT_EQ(L"1.5362", token->m_wstring);

  lexer = pdfium::MakeUnique<CXFA_FMLexer>(L"0.875", nullptr);
  token = lexer->NextToken();
  EXPECT_EQ(TOKnumber, token->m_type);
  EXPECT_EQ(L"0.875", token->m_wstring);

  lexer = pdfium::MakeUnique<CXFA_FMLexer>(L"5.56e-2", nullptr);
  token = lexer->NextToken();
  EXPECT_EQ(TOKnumber, token->m_type);
  EXPECT_EQ(L"5.56e-2", token->m_wstring);

  lexer = pdfium::MakeUnique<CXFA_FMLexer>(L"1.234E10", nullptr);
  token = lexer->NextToken();
  EXPECT_EQ(TOKnumber, token->m_type);
  EXPECT_EQ(L"1.234E10", token->m_wstring);

  lexer = pdfium::MakeUnique<CXFA_FMLexer>(L"123456789.012345678", nullptr);
  token = lexer->NextToken();
  EXPECT_EQ(TOKnumber, token->m_type);
  // TODO(dsinclair): This should round as per IEEE 64-bit values.
  // EXPECT_EQ(L"123456789.01234567", token->m_wstring);
  EXPECT_EQ(L"123456789.012345678", token->m_wstring);

  lexer = pdfium::MakeUnique<CXFA_FMLexer>(L"99999999999999999", nullptr);
  token = lexer->NextToken();
  EXPECT_EQ(TOKnumber, token->m_type);
  // TODO(dsinclair): This is spec'd as rounding when > 16 significant digits
  // prior to the exponent.
  // EXPECT_EQ(L"100000000000000000", token->m_wstring);
  EXPECT_EQ(L"99999999999999999", token->m_wstring);
}

// The quotes are stripped in CXFA_FMStringExpression::ToJavaScript.
TEST(CXFA_FMLexerTest, Strings) {
  auto lexer = pdfium::MakeUnique<CXFA_FMLexer>(
      L"\"The cat jumped over the fence.\"", nullptr);
  CXFA_FMToken* token = lexer->NextToken();
  EXPECT_EQ(TOKstring, token->m_type);
  EXPECT_EQ(L"\"The cat jumped over the fence.\"", token->m_wstring);

  lexer = pdfium::MakeUnique<CXFA_FMLexer>(L"\"\"", nullptr);
  token = lexer->NextToken();
  EXPECT_EQ(TOKstring, token->m_type);
  EXPECT_EQ(L"\"\"", token->m_wstring);

  lexer = pdfium::MakeUnique<CXFA_FMLexer>(
      L"\"The message reads: \"\"Warning: Insufficient Memory\"\"\"", nullptr);
  token = lexer->NextToken();
  EXPECT_EQ(TOKstring, token->m_type);
  EXPECT_EQ(L"\"The message reads: \"\"Warning: Insufficient Memory\"\"\"",
            token->m_wstring);

  lexer = pdfium::MakeUnique<CXFA_FMLexer>(
      L"\"\\u0047\\u006f\\u0066\\u0069\\u0073\\u0068\\u0021\\u000d\\u000a\"",
      nullptr);
  token = lexer->NextToken();
  EXPECT_EQ(TOKstring, token->m_type);
  EXPECT_EQ(
      L"\"\\u0047\\u006f\\u0066\\u0069\\u0073\\u0068\\u0021\\u000d\\u000a\"",
      token->m_wstring);
}

TEST(CXFA_FMLexerTest, Operators) {
  struct {
    const wchar_t* op;
    XFA_FM_TOKEN token;
  } op[] = {
      {L"+", TOKplus},  {L"/", TOKdiv},    {L"==", TOKeq},   {L"eq", TOKkseq},
      {L"<>", TOKne},   {L"ne", TOKksne},  {L"&", TOKand},   {L"and", TOKksand},
      {L"|", TOKor},    {L"or", TOKksor},  {L"*", TOKmul},   {L"<", TOKlt},
      {L"lt", TOKkslt}, {L">", TOKgt},     {L"gt", TOKksgt}, {L"<=", TOKle},
      {L"le", TOKksle}, {L">=", TOKge},    {L"ge", TOKksge}, {L"-", TOKminus},
      {L"+", TOKplus},  {L"not", TOKksnot}};

  for (size_t i = 0; i < FX_ArraySize(op); ++i) {
    auto lexer = pdfium::MakeUnique<CXFA_FMLexer>(op[i].op, nullptr);
    CXFA_FMToken* token = lexer->NextToken();
    EXPECT_EQ(op[i].token, token->m_type);
  }
}
