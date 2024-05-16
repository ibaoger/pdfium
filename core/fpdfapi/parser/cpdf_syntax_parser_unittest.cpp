// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined(UNSAFE_BUFFERS_BUILD)
// TODO(crbug.com/pdfium/2154): resolve buffer safety issues.
#pragma allow_unsafe_buffers
#endif

#include <limits>

#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/cpdf_syntax_parser.h"
#include "core/fxcrt/cfx_read_only_span_stream.h"
#include "core/fxcrt/fx_extension.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/path_service.h"

TEST(SyntaxParserTest, ReadHexString) {
  {
    // Empty string.
    static const uint8_t data[] = "";
    CPDF_SyntaxParser parser(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
        pdfium::make_span(data, 0u)));
    EXPECT_EQ("", parser.ReadHexString());
    EXPECT_EQ(0, parser.GetPos());
  }

  {
    // Blank string.
    static const uint8_t data[] = "  ";
    CPDF_SyntaxParser parser(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
        pdfium::make_span(data, 2u)));
    EXPECT_EQ("", parser.ReadHexString());
    EXPECT_EQ(2, parser.GetPos());
  }

  {
    // Skips unknown characters.
    static const uint8_t data[] = "z12b";
    CPDF_SyntaxParser parser(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
        pdfium::make_span(data, 4u)));
    EXPECT_EQ("\x12\xb0", parser.ReadHexString());
    EXPECT_EQ(4, parser.GetPos());
  }

  {
    // Skips unknown characters.
    static const uint8_t data[] = "*<&*#$^&@1";
    CPDF_SyntaxParser parser(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
        pdfium::make_span(data, 10u)));
    EXPECT_EQ("\x10", parser.ReadHexString());
    EXPECT_EQ(10, parser.GetPos());
  }

  {
    // Skips unknown characters.
    static const uint8_t data[] = "\x80zab";
    CPDF_SyntaxParser parser(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
        pdfium::make_span(data, 4u)));
    EXPECT_EQ("\xab", parser.ReadHexString());
    EXPECT_EQ(4, parser.GetPos());
  }

  {
    // Skips unknown characters.
    static const uint8_t data[] = "\xffzab";
    CPDF_SyntaxParser parser(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
        pdfium::make_span(data, 4u)));
    EXPECT_EQ("\xab", parser.ReadHexString());
    EXPECT_EQ(4, parser.GetPos());
  }

  {
    // Regular conversion.
    static const uint8_t data[] = "1A2b>abcd";
    CPDF_SyntaxParser parser(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
        pdfium::make_span(data, 9u)));
    EXPECT_EQ("\x1a\x2b", parser.ReadHexString());
    EXPECT_EQ(5, parser.GetPos());
  }

  {
    // Position out of bounds.
    static const uint8_t data[] = "12ab>";
    CPDF_SyntaxParser parser(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
        pdfium::make_span(data, 5u)));
    parser.SetPos(5);
    EXPECT_EQ("", parser.ReadHexString());

    parser.SetPos(6);
    EXPECT_EQ("", parser.ReadHexString());

    parser.SetPos(std::numeric_limits<FX_FILESIZE>::max());
    EXPECT_EQ("", parser.ReadHexString());

    // Check string still parses when set to 0.
    parser.SetPos(0);
    EXPECT_EQ("\x12\xab", parser.ReadHexString());
  }

  {
    // Missing ending >.
    static const uint8_t data[] = "1A2b";
    CPDF_SyntaxParser parser(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
        pdfium::make_span(data, 4u)));
    EXPECT_EQ("\x1a\x2b", parser.ReadHexString());
    EXPECT_EQ(4, parser.GetPos());
  }

  {
    // Missing ending >.
    static const uint8_t data[] = "12abz";
    CPDF_SyntaxParser parser(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
        pdfium::make_span(data, 5u)));
    EXPECT_EQ("\x12\xab", parser.ReadHexString());
    EXPECT_EQ(5, parser.GetPos());
  }

  {
    // Uneven number of bytes.
    static const uint8_t data[] = "1A2>asdf";
    CPDF_SyntaxParser parser(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
        pdfium::make_span(data, 8u)));
    EXPECT_EQ("\x1a\x20", parser.ReadHexString());
    EXPECT_EQ(4, parser.GetPos());
  }

  {
    // Uneven number of bytes.
    static const uint8_t data[] = "1A2zasdf";
    CPDF_SyntaxParser parser(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
        pdfium::make_span(data, 8u)));
    EXPECT_EQ("\x1a\x2a\xdf", parser.ReadHexString());
    EXPECT_EQ(8, parser.GetPos());
  }

  {
    // Just ending character.
    const char gt = '>';
    CPDF_SyntaxParser parser(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
        pdfium::byte_span_from_ref(gt)));
    EXPECT_EQ("", parser.ReadHexString());
    EXPECT_EQ(1, parser.GetPos());
  }
}

TEST(SyntaxParserTest, GetInvalidReference) {
  // Data with a reference with number CPDF_Object::kInvalidObjNum
  static const uint8_t data[] = "4294967295 0 R";
  CPDF_SyntaxParser parser(
      pdfium::MakeRetain<CFX_ReadOnlySpanStream>(pdfium::make_span(data, 14u)));
  RetainPtr<CPDF_Object> ref = parser.GetObjectBody(nullptr);
  EXPECT_FALSE(ref);
}

TEST(SyntaxParserTest, PeekNextWord) {
  static const uint8_t data[] = "    WORD ";
  CPDF_SyntaxParser parser(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(data));
  EXPECT_EQ("WORD", parser.PeekNextWord());
  EXPECT_EQ("WORD", parser.GetNextWord().word);
}
