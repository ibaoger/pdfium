// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/widestring.h"

#include <algorithm>
#include <vector>

#include "core/fxcrt/fx_string.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace fxcrt {

TEST(WideString, ElementAccess) {
  const WideString abc(L"abc");
  EXPECT_EQ(L'a', abc[0]);
  EXPECT_EQ(L'b', abc[1]);
  EXPECT_EQ(L'c', abc[2]);
#ifndef NDEBUG
  EXPECT_DEATH({ abc[4]; }, ".*");
#endif

  WideString mutable_abc = abc;
  EXPECT_EQ(abc.c_str(), mutable_abc.c_str());
  EXPECT_EQ(L'a', mutable_abc[0]);
  EXPECT_EQ(L'b', mutable_abc[1]);
  EXPECT_EQ(L'c', mutable_abc[2]);
  EXPECT_EQ(abc.c_str(), mutable_abc.c_str());
  EXPECT_EQ(L"abc", abc);

  const wchar_t* c_str = abc.c_str();
  mutable_abc.SetAt(0, L'd');
  EXPECT_EQ(c_str, abc.c_str());
  EXPECT_NE(c_str, mutable_abc.c_str());
  EXPECT_EQ(L"abc", abc);
  EXPECT_EQ(L"dbc", mutable_abc);

  mutable_abc.SetAt(1, L'e');
  EXPECT_EQ(L"abc", abc);
  EXPECT_EQ(L"dec", mutable_abc);

  mutable_abc.SetAt(2, L'f');
  EXPECT_EQ(L"abc", abc);
  EXPECT_EQ(L"def", mutable_abc);
#ifndef NDEBUG
  EXPECT_DEATH({ mutable_abc.SetAt(3, L'g'); }, ".*");
  EXPECT_EQ(L"abc", abc);
#endif
}

TEST(WideString, OperatorLT) {
  WideString empty;
  WideString a(L"a");
  WideString abc(L"\x0110qq");  // Comes before despite endianness.
  WideString def(L"\x1001qq");  // Comes after despite endianness.

  EXPECT_FALSE(empty < empty);
  EXPECT_FALSE(a < a);
  EXPECT_FALSE(abc < abc);
  EXPECT_FALSE(def < def);

  EXPECT_TRUE(empty < a);
  EXPECT_FALSE(a < empty);

  EXPECT_TRUE(empty < abc);
  EXPECT_FALSE(abc < empty);

  EXPECT_TRUE(empty < def);
  EXPECT_FALSE(def < empty);

  EXPECT_TRUE(a < abc);
  EXPECT_FALSE(abc < a);

  EXPECT_TRUE(a < def);
  EXPECT_FALSE(def < a);

  EXPECT_TRUE(abc < def);
  EXPECT_FALSE(def < abc);
}

TEST(WideString, OperatorEQ) {
  WideString null_string;
  EXPECT_TRUE(null_string == null_string);

  WideString empty_string(L"");
  EXPECT_TRUE(empty_string == empty_string);
  EXPECT_TRUE(empty_string == null_string);
  EXPECT_TRUE(null_string == empty_string);

  WideString deleted_string(L"hello");
  deleted_string.Delete(0, 5);
  EXPECT_TRUE(deleted_string == deleted_string);
  EXPECT_TRUE(deleted_string == null_string);
  EXPECT_TRUE(deleted_string == empty_string);
  EXPECT_TRUE(null_string == deleted_string);
  EXPECT_TRUE(null_string == empty_string);

  WideString wide_string(L"hello");
  EXPECT_TRUE(wide_string == wide_string);
  EXPECT_FALSE(wide_string == null_string);
  EXPECT_FALSE(wide_string == empty_string);
  EXPECT_FALSE(wide_string == deleted_string);
  EXPECT_FALSE(null_string == wide_string);
  EXPECT_FALSE(empty_string == wide_string);
  EXPECT_FALSE(deleted_string == wide_string);

  WideString wide_string_same1(L"hello");
  EXPECT_TRUE(wide_string == wide_string_same1);
  EXPECT_TRUE(wide_string_same1 == wide_string);

  WideString wide_string_same2(wide_string);
  EXPECT_TRUE(wide_string == wide_string_same2);
  EXPECT_TRUE(wide_string_same2 == wide_string);

  WideString wide_string1(L"he");
  WideString wide_string2(L"hellp");
  WideString wide_string3(L"hellod");
  EXPECT_FALSE(wide_string == wide_string1);
  EXPECT_FALSE(wide_string == wide_string2);
  EXPECT_FALSE(wide_string == wide_string3);
  EXPECT_FALSE(wide_string1 == wide_string);
  EXPECT_FALSE(wide_string2 == wide_string);
  EXPECT_FALSE(wide_string3 == wide_string);

  WideStringView null_string_c;
  WideStringView empty_string_c(L"");
  EXPECT_TRUE(null_string == null_string_c);
  EXPECT_TRUE(null_string == empty_string_c);
  EXPECT_TRUE(empty_string == null_string_c);
  EXPECT_TRUE(empty_string == empty_string_c);
  EXPECT_TRUE(deleted_string == null_string_c);
  EXPECT_TRUE(deleted_string == empty_string_c);
  EXPECT_TRUE(null_string_c == null_string);
  EXPECT_TRUE(empty_string_c == null_string);
  EXPECT_TRUE(null_string_c == empty_string);
  EXPECT_TRUE(empty_string_c == empty_string);
  EXPECT_TRUE(null_string_c == deleted_string);
  EXPECT_TRUE(empty_string_c == deleted_string);

  WideStringView wide_string_c_same1(L"hello");
  EXPECT_TRUE(wide_string == wide_string_c_same1);
  EXPECT_TRUE(wide_string_c_same1 == wide_string);

  WideStringView wide_string_c1(L"he");
  WideStringView wide_string_c2(L"hellp");
  WideStringView wide_string_c3(L"hellod");
  EXPECT_FALSE(wide_string == wide_string_c1);
  EXPECT_FALSE(wide_string == wide_string_c2);
  EXPECT_FALSE(wide_string == wide_string_c3);
  EXPECT_FALSE(wide_string_c1 == wide_string);
  EXPECT_FALSE(wide_string_c2 == wide_string);
  EXPECT_FALSE(wide_string_c3 == wide_string);

  const wchar_t* c_null_string = nullptr;
  const wchar_t* c_empty_string = L"";
  EXPECT_TRUE(null_string == c_null_string);
  EXPECT_TRUE(null_string == c_empty_string);
  EXPECT_TRUE(empty_string == c_null_string);
  EXPECT_TRUE(empty_string == c_empty_string);
  EXPECT_TRUE(deleted_string == c_null_string);
  EXPECT_TRUE(deleted_string == c_empty_string);
  EXPECT_TRUE(c_null_string == null_string);
  EXPECT_TRUE(c_empty_string == null_string);
  EXPECT_TRUE(c_null_string == empty_string);
  EXPECT_TRUE(c_empty_string == empty_string);
  EXPECT_TRUE(c_null_string == deleted_string);
  EXPECT_TRUE(c_empty_string == deleted_string);

  const wchar_t* c_string_same1 = L"hello";
  EXPECT_TRUE(wide_string == c_string_same1);
  EXPECT_TRUE(c_string_same1 == wide_string);

  const wchar_t* c_string1 = L"he";
  const wchar_t* c_string2 = L"hellp";
  const wchar_t* c_string3 = L"hellod";
  EXPECT_FALSE(wide_string == c_string1);
  EXPECT_FALSE(wide_string == c_string2);
  EXPECT_FALSE(wide_string == c_string3);
  EXPECT_FALSE(c_string1 == wide_string);
  EXPECT_FALSE(c_string2 == wide_string);
  EXPECT_FALSE(c_string3 == wide_string);
}

TEST(WideString, OperatorNE) {
  WideString null_string;
  EXPECT_FALSE(null_string != null_string);

  WideString empty_string(L"");
  EXPECT_FALSE(empty_string != empty_string);
  EXPECT_FALSE(empty_string != null_string);
  EXPECT_FALSE(null_string != empty_string);

  WideString deleted_string(L"hello");
  deleted_string.Delete(0, 5);
  EXPECT_FALSE(deleted_string != deleted_string);
  EXPECT_FALSE(deleted_string != null_string);
  EXPECT_FALSE(deleted_string != empty_string);
  EXPECT_FALSE(null_string != deleted_string);
  EXPECT_FALSE(null_string != empty_string);

  WideString wide_string(L"hello");
  EXPECT_FALSE(wide_string != wide_string);
  EXPECT_TRUE(wide_string != null_string);
  EXPECT_TRUE(wide_string != empty_string);
  EXPECT_TRUE(wide_string != deleted_string);
  EXPECT_TRUE(null_string != wide_string);
  EXPECT_TRUE(empty_string != wide_string);
  EXPECT_TRUE(deleted_string != wide_string);

  WideString wide_string_same1(L"hello");
  EXPECT_FALSE(wide_string != wide_string_same1);
  EXPECT_FALSE(wide_string_same1 != wide_string);

  WideString wide_string_same2(wide_string);
  EXPECT_FALSE(wide_string != wide_string_same2);
  EXPECT_FALSE(wide_string_same2 != wide_string);

  WideString wide_string1(L"he");
  WideString wide_string2(L"hellp");
  WideString wide_string3(L"hellod");
  EXPECT_TRUE(wide_string != wide_string1);
  EXPECT_TRUE(wide_string != wide_string2);
  EXPECT_TRUE(wide_string != wide_string3);
  EXPECT_TRUE(wide_string1 != wide_string);
  EXPECT_TRUE(wide_string2 != wide_string);
  EXPECT_TRUE(wide_string3 != wide_string);

  WideStringView null_string_c;
  WideStringView empty_string_c(L"");
  EXPECT_FALSE(null_string != null_string_c);
  EXPECT_FALSE(null_string != empty_string_c);
  EXPECT_FALSE(empty_string != null_string_c);
  EXPECT_FALSE(empty_string != empty_string_c);
  EXPECT_FALSE(deleted_string != null_string_c);
  EXPECT_FALSE(deleted_string != empty_string_c);
  EXPECT_FALSE(null_string_c != null_string);
  EXPECT_FALSE(empty_string_c != null_string);
  EXPECT_FALSE(null_string_c != empty_string);
  EXPECT_FALSE(empty_string_c != empty_string);

  WideStringView wide_string_c_same1(L"hello");
  EXPECT_FALSE(wide_string != wide_string_c_same1);
  EXPECT_FALSE(wide_string_c_same1 != wide_string);

  WideStringView wide_string_c1(L"he");
  WideStringView wide_string_c2(L"hellp");
  WideStringView wide_string_c3(L"hellod");
  EXPECT_TRUE(wide_string != wide_string_c1);
  EXPECT_TRUE(wide_string != wide_string_c2);
  EXPECT_TRUE(wide_string != wide_string_c3);
  EXPECT_TRUE(wide_string_c1 != wide_string);
  EXPECT_TRUE(wide_string_c2 != wide_string);
  EXPECT_TRUE(wide_string_c3 != wide_string);

  const wchar_t* c_null_string = nullptr;
  const wchar_t* c_empty_string = L"";
  EXPECT_FALSE(null_string != c_null_string);
  EXPECT_FALSE(null_string != c_empty_string);
  EXPECT_FALSE(empty_string != c_null_string);
  EXPECT_FALSE(empty_string != c_empty_string);
  EXPECT_FALSE(deleted_string != c_null_string);
  EXPECT_FALSE(deleted_string != c_empty_string);
  EXPECT_FALSE(c_null_string != null_string);
  EXPECT_FALSE(c_empty_string != null_string);
  EXPECT_FALSE(c_null_string != empty_string);
  EXPECT_FALSE(c_empty_string != empty_string);
  EXPECT_FALSE(c_null_string != deleted_string);
  EXPECT_FALSE(c_empty_string != deleted_string);

  const wchar_t* c_string_same1 = L"hello";
  EXPECT_FALSE(wide_string != c_string_same1);
  EXPECT_FALSE(c_string_same1 != wide_string);

  const wchar_t* c_string1 = L"he";
  const wchar_t* c_string2 = L"hellp";
  const wchar_t* c_string3 = L"hellod";
  EXPECT_TRUE(wide_string != c_string1);
  EXPECT_TRUE(wide_string != c_string2);
  EXPECT_TRUE(wide_string != c_string3);
  EXPECT_TRUE(c_string1 != wide_string);
  EXPECT_TRUE(c_string2 != wide_string);
  EXPECT_TRUE(c_string3 != wide_string);
}

TEST(WideString, ConcatInPlace) {
  WideString fred;
  fred.Concat(L"FRED", 4);
  EXPECT_EQ(L"FRED", fred);

  fred.Concat(L"DY", 2);
  EXPECT_EQ(L"FREDDY", fred);

  fred.Delete(3, 3);
  EXPECT_EQ(L"FRE", fred);

  fred.Concat(L"D", 1);
  EXPECT_EQ(L"FRED", fred);

  WideString copy = fred;
  fred.Concat(L"DY", 2);
  EXPECT_EQ(L"FREDDY", fred);
  EXPECT_EQ(L"FRED", copy);
}

TEST(WideString, Remove) {
  WideString freed(L"FREED");
  freed.Remove(L'E');
  EXPECT_EQ(L"FRD", freed);
  freed.Remove(L'F');
  EXPECT_EQ(L"RD", freed);
  freed.Remove(L'D');
  EXPECT_EQ(L"R", freed);
  freed.Remove(L'X');
  EXPECT_EQ(L"R", freed);
  freed.Remove(L'R');
  EXPECT_EQ(L"", freed);

  WideString empty;
  empty.Remove(L'X');
  EXPECT_EQ(L"", empty);
}

TEST(WideString, RemoveCopies) {
  WideString freed(L"FREED");
  const wchar_t* old_buffer = freed.c_str();

  // No change with single reference - no copy.
  freed.Remove(L'Q');
  EXPECT_EQ(L"FREED", freed);
  EXPECT_EQ(old_buffer, freed.c_str());

  // Change with single reference - no copy.
  freed.Remove(L'E');
  EXPECT_EQ(L"FRD", freed);
  EXPECT_EQ(old_buffer, freed.c_str());

  // No change with multiple references - no copy.
  WideString shared(freed);
  freed.Remove(L'Q');
  EXPECT_EQ(L"FRD", freed);
  EXPECT_EQ(old_buffer, freed.c_str());
  EXPECT_EQ(old_buffer, shared.c_str());

  // Change with multiple references -- must copy.
  freed.Remove(L'D');
  EXPECT_EQ(L"FR", freed);
  EXPECT_NE(old_buffer, freed.c_str());
  EXPECT_EQ(L"FRD", shared);
  EXPECT_EQ(old_buffer, shared.c_str());
}

TEST(WideString, Replace) {
  WideString fred(L"FRED");
  fred.Replace(L"FR", L"BL");
  EXPECT_EQ(L"BLED", fred);
  fred.Replace(L"D", L"DDY");
  EXPECT_EQ(L"BLEDDY", fred);
  fred.Replace(L"LEDD", L"");
  EXPECT_EQ(L"BY", fred);
  fred.Replace(L"X", L"CLAMS");
  EXPECT_EQ(L"BY", fred);
  fred.Replace(L"BY", L"HI");
  EXPECT_EQ(L"HI", fred);
  fred.Replace(L"", L"CLAMS");
  EXPECT_EQ(L"HI", fred);
  fred.Replace(L"HI", L"");
  EXPECT_EQ(L"", fred);
}

TEST(WideString, Insert) {
  WideString fred(L"FRED");
  EXPECT_EQ(5u, fred.Insert(0, 'S'));
  EXPECT_EQ(L"SFRED", fred);
  EXPECT_EQ(6u, fred.Insert(1, 'T'));
  EXPECT_EQ(L"STFRED", fred);
  EXPECT_EQ(7u, fred.Insert(4, 'U'));
  EXPECT_EQ(L"STFRUED", fred);
  EXPECT_EQ(8u, fred.Insert(7, 'V'));
  EXPECT_EQ(L"STFRUEDV", fred);
  EXPECT_EQ(8u, fred.Insert(12, 'P'));
  EXPECT_EQ(L"STFRUEDV", fred);
  {
    WideString empty;
    EXPECT_EQ(1u, empty.Insert(0, 'X'));
    EXPECT_EQ(L"X", empty);
  }
  {
    WideString empty;
    EXPECT_EQ(0u, empty.Insert(5, 'X'));
    EXPECT_NE(L"X", empty);
  }
}

TEST(WideString, InsertAtFrontAndInsertAtBack) {
  {
    WideString empty;
    EXPECT_EQ(1u, empty.InsertAtFront('D'));
    EXPECT_EQ(L"D", empty);
    EXPECT_EQ(2u, empty.InsertAtFront('E'));
    EXPECT_EQ(L"ED", empty);
    EXPECT_EQ(3u, empty.InsertAtFront('R'));
    EXPECT_EQ(L"RED", empty);
    EXPECT_EQ(4u, empty.InsertAtFront('F'));
    EXPECT_EQ(L"FRED", empty);
  }
  {
    WideString empty;
    EXPECT_EQ(1u, empty.InsertAtBack('F'));
    EXPECT_EQ(L"F", empty);
    EXPECT_EQ(2u, empty.InsertAtBack('R'));
    EXPECT_EQ(L"FR", empty);
    EXPECT_EQ(3u, empty.InsertAtBack('E'));
    EXPECT_EQ(L"FRE", empty);
    EXPECT_EQ(4u, empty.InsertAtBack('D'));
    EXPECT_EQ(L"FRED", empty);
  }
  {
    WideString empty;
    EXPECT_EQ(1u, empty.InsertAtBack('E'));
    EXPECT_EQ(L"E", empty);
    EXPECT_EQ(2u, empty.InsertAtFront('R'));
    EXPECT_EQ(L"RE", empty);
    EXPECT_EQ(3u, empty.InsertAtBack('D'));
    EXPECT_EQ(L"RED", empty);
    EXPECT_EQ(4u, empty.InsertAtFront('F'));
    EXPECT_EQ(L"FRED", empty);
  }
}

TEST(WideString, Delete) {
  WideString fred(L"FRED");
  EXPECT_EQ(4u, fred.Delete(0, 0));
  EXPECT_EQ(L"FRED", fred);
  EXPECT_EQ(2u, fred.Delete(0, 2));
  EXPECT_EQ(L"ED", fred);
  EXPECT_EQ(1u, fred.Delete(1));
  EXPECT_EQ(L"E", fred);
  EXPECT_EQ(0u, fred.Delete(0));
  EXPECT_EQ(L"", fred);
  EXPECT_EQ(0u, fred.Delete(0));
  EXPECT_EQ(L"", fred);

  WideString empty;
  EXPECT_EQ(0u, empty.Delete(0));
  EXPECT_EQ(L"", empty);
  EXPECT_EQ(0u, empty.Delete(1));
  EXPECT_EQ(L"", empty);
}

TEST(WideString, Mid) {
  WideString fred(L"FRED");
  EXPECT_EQ(L"", fred.Mid(0, 0));
  EXPECT_EQ(L"", fred.Mid(3, 0));
  EXPECT_EQ(L"FRED", fred.Mid(0, 4));
  EXPECT_EQ(L"RED", fred.Mid(1, 3));
  EXPECT_EQ(L"ED", fred.Mid(2, 2));
  EXPECT_EQ(L"D", fred.Mid(3, 1));
  EXPECT_EQ(L"F", fred.Mid(0, 1));
  EXPECT_EQ(L"R", fred.Mid(1, 1));
  EXPECT_EQ(L"E", fred.Mid(2, 1));
  EXPECT_EQ(L"D", fred.Mid(3, 1));
  EXPECT_EQ(L"FR", fred.Mid(0, 2));
  EXPECT_EQ(L"FRED", fred.Mid(0, 4));
  EXPECT_EQ(L"", fred.Mid(0, 10));

  EXPECT_EQ(L"", fred.Mid(1, 4));
  EXPECT_EQ(L"", fred.Mid(4, 1));

  WideString empty;
  EXPECT_EQ(L"", empty.Mid(0, 0));
}

TEST(WideString, Left) {
  WideString fred(L"FRED");
  EXPECT_EQ(L"", fred.Left(0));
  EXPECT_EQ(L"F", fred.Left(1));
  EXPECT_EQ(L"FR", fred.Left(2));
  EXPECT_EQ(L"FRE", fred.Left(3));
  EXPECT_EQ(L"FRED", fred.Left(4));

  EXPECT_EQ(L"", fred.Left(5));

  WideString empty;
  EXPECT_EQ(L"", empty.Left(0));
  EXPECT_EQ(L"", empty.Left(1));
}

TEST(WideString, Right) {
  WideString fred(L"FRED");
  EXPECT_EQ(L"", fred.Right(0));
  EXPECT_EQ(L"D", fred.Right(1));
  EXPECT_EQ(L"ED", fred.Right(2));
  EXPECT_EQ(L"RED", fred.Right(3));
  EXPECT_EQ(L"FRED", fred.Right(4));

  EXPECT_EQ(L"", fred.Right(5));

  WideString empty;
  EXPECT_EQ(L"", empty.Right(0));
  EXPECT_EQ(L"", empty.Right(1));
}

TEST(WideString, Find) {
  WideString null_string;
  EXPECT_FALSE(null_string.Find(L'a').has_value());
  EXPECT_FALSE(null_string.Find(L'\0').has_value());

  WideString empty_string(L"");
  EXPECT_FALSE(empty_string.Find(L'a').has_value());
  EXPECT_FALSE(empty_string.Find(L'\0').has_value());

  pdfium::Optional<size_t> result;
  WideString single_string(L"a");
  result = single_string.Find(L'a');
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(0u, result.value());
  EXPECT_FALSE(single_string.Find(L'b').has_value());
  EXPECT_FALSE(single_string.Find(L'\0').has_value());

  WideString longer_string(L"abccc");
  result = longer_string.Find(L'a');
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(0u, result.value());
  result = longer_string.Find(L'c');
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(2u, result.value());
  result = longer_string.Find(L'c', 3);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(3u, result.value());
  EXPECT_FALSE(longer_string.Find(L'\0').has_value());

  result = longer_string.Find(L"ab");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(0u, result.value());
  result = longer_string.Find(L"ccc");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(2u, result.value());
  result = longer_string.Find(L"cc", 3);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(3u, result.value());
  EXPECT_FALSE(longer_string.Find(L"d").has_value());

  WideString hibyte_string(
      L"ab\xff8c"
      L"def");
  result = hibyte_string.Find(L'\xff8c');
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(2u, result.value());
}

TEST(WideString, UpperLower) {
  WideString fred(L"F-Re.42D");
  fred.MakeLower();
  EXPECT_EQ(L"f-re.42d", fred);
  fred.MakeUpper();
  EXPECT_EQ(L"F-RE.42D", fred);

  WideString empty;
  empty.MakeLower();
  EXPECT_EQ(L"", empty);
  empty.MakeUpper();
  EXPECT_EQ(L"", empty);
}

TEST(WideString, TrimRight) {
  WideString fred(L"  FRED  ");
  fred.TrimRight();
  EXPECT_EQ(L"  FRED", fred);
  fred.TrimRight(L'E');
  EXPECT_EQ(L"  FRED", fred);
  fred.TrimRight(L'D');
  EXPECT_EQ(L"  FRE", fred);
  fred.TrimRight(L"ERP");
  EXPECT_EQ(L"  F", fred);

  WideString blank(L"   ");
  blank.TrimRight(L"ERP");
  EXPECT_EQ(L"   ", blank);
  blank.TrimRight(L'E');
  EXPECT_EQ(L"   ", blank);
  blank.TrimRight();
  EXPECT_EQ(L"", blank);

  WideString empty;
  empty.TrimRight(L"ERP");
  EXPECT_EQ(L"", empty);
  empty.TrimRight(L'E');
  EXPECT_EQ(L"", empty);
  empty.TrimRight();
  EXPECT_EQ(L"", empty);
}

TEST(WideString, TrimRightCopies) {
  {
    // With a single reference, no copy takes place.
    WideString fred(L"  FRED  ");
    const wchar_t* old_buffer = fred.c_str();
    fred.TrimRight();
    EXPECT_EQ(L"  FRED", fred);
    EXPECT_EQ(old_buffer, fred.c_str());
  }
  {
    // With multiple references, we must copy.
    WideString fred(L"  FRED  ");
    WideString other_fred = fred;
    const wchar_t* old_buffer = fred.c_str();
    fred.TrimRight();
    EXPECT_EQ(L"  FRED", fred);
    EXPECT_EQ(L"  FRED  ", other_fred);
    EXPECT_NE(old_buffer, fred.c_str());
  }
  {
    // With multiple references, but no modifications, no copy.
    WideString fred(L"FRED");
    WideString other_fred = fred;
    const wchar_t* old_buffer = fred.c_str();
    fred.TrimRight();
    EXPECT_EQ(L"FRED", fred);
    EXPECT_EQ(L"FRED", other_fred);
    EXPECT_EQ(old_buffer, fred.c_str());
  }
}

TEST(WideString, TrimLeft) {
  WideString fred(L"  FRED  ");
  fred.TrimLeft();
  EXPECT_EQ(L"FRED  ", fred);
  fred.TrimLeft(L'E');
  EXPECT_EQ(L"FRED  ", fred);
  fred.TrimLeft(L'F');
  EXPECT_EQ(L"RED  ", fred);
  fred.TrimLeft(L"ERP");
  EXPECT_EQ(L"D  ", fred);

  WideString blank(L"   ");
  blank.TrimLeft(L"ERP");
  EXPECT_EQ(L"   ", blank);
  blank.TrimLeft(L'E');
  EXPECT_EQ(L"   ", blank);
  blank.TrimLeft();
  EXPECT_EQ(L"", blank);

  WideString empty;
  empty.TrimLeft(L"ERP");
  EXPECT_EQ(L"", empty);
  empty.TrimLeft(L'E');
  EXPECT_EQ(L"", empty);
  empty.TrimLeft();
  EXPECT_EQ(L"", empty);
}

TEST(WideString, TrimLeftCopies) {
  {
    // With a single reference, no copy takes place.
    WideString fred(L"  FRED  ");
    const wchar_t* old_buffer = fred.c_str();
    fred.TrimLeft();
    EXPECT_EQ(L"FRED  ", fred);
    EXPECT_EQ(old_buffer, fred.c_str());
  }
  {
    // With multiple references, we must copy.
    WideString fred(L"  FRED  ");
    WideString other_fred = fred;
    const wchar_t* old_buffer = fred.c_str();
    fred.TrimLeft();
    EXPECT_EQ(L"FRED  ", fred);
    EXPECT_EQ(L"  FRED  ", other_fred);
    EXPECT_NE(old_buffer, fred.c_str());
  }
  {
    // With multiple references, but no modifications, no copy.
    WideString fred(L"FRED");
    WideString other_fred = fred;
    const wchar_t* old_buffer = fred.c_str();
    fred.TrimLeft();
    EXPECT_EQ(L"FRED", fred);
    EXPECT_EQ(L"FRED", other_fred);
    EXPECT_EQ(old_buffer, fred.c_str());
  }
}

TEST(WideString, Reserve) {
  {
    WideString str;
    str.Reserve(6);
    const wchar_t* old_buffer = str.c_str();
    str += L"ABCDEF";
    EXPECT_EQ(old_buffer, str.c_str());
    str += L"Blah Blah Blah Blah Blah Blah";
    EXPECT_NE(old_buffer, str.c_str());
  }
  {
    WideString str(L"A");
    str.Reserve(6);
    const wchar_t* old_buffer = str.c_str();
    str += L"BCDEF";
    EXPECT_EQ(old_buffer, str.c_str());
    str += L"Blah Blah Blah Blah Blah Blah";
    EXPECT_NE(old_buffer, str.c_str());
  }
}

TEST(WideString, GetBuffer) {
  {
    WideString str;
    wchar_t* buffer = str.GetBuffer(12);
    wcscpy(buffer, L"clams");
    str.ReleaseBuffer(str.GetStringLength());
    EXPECT_EQ(L"clams", str);
  }
  {
    WideString str(L"cl");
    wchar_t* buffer = str.GetBuffer(12);
    wcscpy(buffer + 2, L"ams");
    str.ReleaseBuffer(str.GetStringLength());
    EXPECT_EQ(L"clams", str);
  }
}

TEST(WideString, ReleaseBuffer) {
  {
    WideString str;
    str.Reserve(12);
    str += L"clams";
    const wchar_t* old_buffer = str.c_str();
    str.ReleaseBuffer(4);
    EXPECT_EQ(old_buffer, str.c_str());
    EXPECT_EQ(L"clam", str);
  }
  {
    WideString str(L"c");
    str.Reserve(12);
    str += L"lams";
    const wchar_t* old_buffer = str.c_str();
    str.ReleaseBuffer(4);
    EXPECT_EQ(old_buffer, str.c_str());
    EXPECT_EQ(L"clam", str);
  }
  {
    WideString str;
    str.Reserve(200);
    str += L"clams";
    const wchar_t* old_buffer = str.c_str();
    str.ReleaseBuffer(4);
    EXPECT_NE(old_buffer, str.c_str());
    EXPECT_EQ(L"clam", str);
  }
  {
    WideString str(L"c");
    str.Reserve(200);
    str += L"lams";
    const wchar_t* old_buffer = str.c_str();
    str.ReleaseBuffer(4);
    EXPECT_NE(old_buffer, str.c_str());
    EXPECT_EQ(L"clam", str);
  }
}

TEST(WideString, EmptyReverseIterator) {
  WideString empty;
  auto iter = empty.rbegin();
  EXPECT_TRUE(iter == empty.rend());
  EXPECT_FALSE(iter != empty.rend());
  EXPECT_FALSE(iter < empty.rend());
}

TEST(WideString, OneCharReverseIterator) {
  WideString one_str(L"a");
  auto iter = one_str.rbegin();
  EXPECT_FALSE(iter == one_str.rend());
  EXPECT_TRUE(iter != one_str.rend());
  EXPECT_TRUE(iter < one_str.rend());

  char ch = *iter++;
  EXPECT_EQ('a', ch);
  EXPECT_TRUE(iter == one_str.rend());
  EXPECT_FALSE(iter != one_str.rend());
  EXPECT_FALSE(iter < one_str.rend());
}

TEST(WideString, MultiCharReverseIterator) {
  WideString multi_str(L"abcd");
  auto iter = multi_str.rbegin();
  EXPECT_FALSE(iter == multi_str.rend());

  char ch = *iter++;
  EXPECT_EQ('d', ch);
  EXPECT_EQ('c', *iter);
  EXPECT_FALSE(iter == multi_str.rend());

  ch = *(++iter);
  EXPECT_EQ('b', ch);
  EXPECT_EQ('b', *iter);
  EXPECT_FALSE(iter == multi_str.rend());

  ch = *iter++;
  EXPECT_EQ('b', ch);
  EXPECT_EQ('a', *iter);
  EXPECT_FALSE(iter == multi_str.rend());

  ch = *iter++;
  EXPECT_EQ('a', ch);
  EXPECT_TRUE(iter == multi_str.rend());

  ch = *(--iter);
  EXPECT_EQ('a', ch);
  EXPECT_EQ('a', *iter);
  EXPECT_FALSE(iter == multi_str.rend());

  ch = *iter--;
  EXPECT_EQ('a', ch);
  EXPECT_EQ('b', *iter);
  EXPECT_FALSE(iter == multi_str.rend());

  ch = *iter--;
  EXPECT_EQ('b', ch);
  EXPECT_EQ('c', *iter);
  EXPECT_FALSE(iter == multi_str.rend());

  ch = *(--iter);
  EXPECT_EQ('d', ch);
  EXPECT_EQ('d', *iter);
  EXPECT_TRUE(iter == multi_str.rbegin());
}

TEST(WideString, UTF16LE_Encode) {
  struct UTF16LEEncodeCase {
    WideString ws;
    ByteString bs;
  } utf16le_encode_cases[] = {
      {L"", ByteString("\0\0", 2)},
      {L"abc", ByteString("a\0b\0c\0\0\0", 8)},
      {L"abcdef", ByteString("a\0b\0c\0d\0e\0f\0\0\0", 14)},
      {L"abc\0def", ByteString("a\0b\0c\0\0\0", 8)},
      {L"\xaabb\xccdd", ByteString("\xbb\xaa\xdd\xcc\0\0", 6)},
      {L"\x3132\x6162", ByteString("\x32\x31\x62\x61\0\0", 6)},
  };

  for (size_t i = 0; i < FX_ArraySize(utf16le_encode_cases); ++i) {
    EXPECT_EQ(utf16le_encode_cases[i].bs,
              utf16le_encode_cases[i].ws.UTF16LE_Encode())
        << " for case number " << i;
  }
}

TEST(WideStringView, FromVector) {
  std::vector<WideStringView::UnsignedType> null_vec;
  WideStringView null_string(null_vec);
  EXPECT_EQ(0u, null_string.GetLength());

  std::vector<WideStringView::UnsignedType> lower_a_vec(
      10, static_cast<WideStringView::UnsignedType>(L'a'));
  WideStringView lower_a_string(lower_a_vec);
  EXPECT_EQ(10u, lower_a_string.GetLength());
  EXPECT_EQ(L"aaaaaaaaaa", lower_a_string);

  std::vector<WideStringView::UnsignedType> cleared_vec;
  cleared_vec.push_back(42);
  cleared_vec.pop_back();
  WideStringView cleared_string(cleared_vec);
  EXPECT_EQ(0u, cleared_string.GetLength());
  EXPECT_EQ(nullptr, cleared_string.raw_str());
}

TEST(WideStringView, ElementAccess) {
  WideStringView abc(L"abc");
  EXPECT_EQ(L'a', static_cast<wchar_t>(abc[0]));
  EXPECT_EQ(L'b', static_cast<wchar_t>(abc[1]));
  EXPECT_EQ(L'c', static_cast<wchar_t>(abc[2]));
#ifndef NDEBUG
  EXPECT_DEATH({ abc[4]; }, ".*");
#endif
}

TEST(WideStringView, OperatorLT) {
  WideStringView empty;
  WideStringView a(L"a");
  WideStringView abc(L"\x0110qq");  // Comes InsertAtFront despite endianness.
  WideStringView def(L"\x1001qq");  // Comes InsertAtBack despite endianness.

  EXPECT_FALSE(empty < empty);
  EXPECT_FALSE(a < a);
  EXPECT_FALSE(abc < abc);
  EXPECT_FALSE(def < def);

  EXPECT_TRUE(empty < a);
  EXPECT_FALSE(a < empty);

  EXPECT_TRUE(empty < abc);
  EXPECT_FALSE(abc < empty);

  EXPECT_TRUE(empty < def);
  EXPECT_FALSE(def < empty);

  EXPECT_TRUE(a < abc);
  EXPECT_FALSE(abc < a);

  EXPECT_TRUE(a < def);
  EXPECT_FALSE(def < a);

  EXPECT_TRUE(abc < def);
  EXPECT_FALSE(def < abc);
}

TEST(WideStringView, OperatorEQ) {
  WideStringView wide_string_c(L"hello");
  EXPECT_TRUE(wide_string_c == wide_string_c);

  WideStringView wide_string_c_same1(L"hello");
  EXPECT_TRUE(wide_string_c == wide_string_c_same1);
  EXPECT_TRUE(wide_string_c_same1 == wide_string_c);

  WideStringView wide_string_c_same2(wide_string_c);
  EXPECT_TRUE(wide_string_c == wide_string_c_same2);
  EXPECT_TRUE(wide_string_c_same2 == wide_string_c);

  WideStringView wide_string_c1(L"he");
  WideStringView wide_string_c2(L"hellp");
  WideStringView wide_string_c3(L"hellod");
  EXPECT_FALSE(wide_string_c == wide_string_c1);
  EXPECT_FALSE(wide_string_c == wide_string_c2);
  EXPECT_FALSE(wide_string_c == wide_string_c3);
  EXPECT_FALSE(wide_string_c1 == wide_string_c);
  EXPECT_FALSE(wide_string_c2 == wide_string_c);
  EXPECT_FALSE(wide_string_c3 == wide_string_c);

  WideString wide_string_same1(L"hello");
  EXPECT_TRUE(wide_string_c == wide_string_same1);
  EXPECT_TRUE(wide_string_same1 == wide_string_c);

  WideString wide_string1(L"he");
  WideString wide_string2(L"hellp");
  WideString wide_string3(L"hellod");
  EXPECT_FALSE(wide_string_c == wide_string1);
  EXPECT_FALSE(wide_string_c == wide_string2);
  EXPECT_FALSE(wide_string_c == wide_string3);
  EXPECT_FALSE(wide_string1 == wide_string_c);
  EXPECT_FALSE(wide_string2 == wide_string_c);
  EXPECT_FALSE(wide_string3 == wide_string_c);

  const wchar_t* c_string_same1 = L"hello";
  EXPECT_TRUE(wide_string_c == c_string_same1);
  EXPECT_TRUE(c_string_same1 == wide_string_c);

  const wchar_t* c_string1 = L"he";
  const wchar_t* c_string2 = L"hellp";
  const wchar_t* c_string3 = L"hellod";
  EXPECT_FALSE(wide_string_c == c_string1);
  EXPECT_FALSE(wide_string_c == c_string2);
  EXPECT_FALSE(wide_string_c == c_string3);

  EXPECT_FALSE(c_string1 == wide_string_c);
  EXPECT_FALSE(c_string2 == wide_string_c);
  EXPECT_FALSE(c_string3 == wide_string_c);
}

TEST(WideStringView, OperatorNE) {
  WideStringView wide_string_c(L"hello");
  EXPECT_FALSE(wide_string_c != wide_string_c);

  WideStringView wide_string_c_same1(L"hello");
  EXPECT_FALSE(wide_string_c != wide_string_c_same1);
  EXPECT_FALSE(wide_string_c_same1 != wide_string_c);

  WideStringView wide_string_c_same2(wide_string_c);
  EXPECT_FALSE(wide_string_c != wide_string_c_same2);
  EXPECT_FALSE(wide_string_c_same2 != wide_string_c);

  WideStringView wide_string_c1(L"he");
  WideStringView wide_string_c2(L"hellp");
  WideStringView wide_string_c3(L"hellod");
  EXPECT_TRUE(wide_string_c != wide_string_c1);
  EXPECT_TRUE(wide_string_c != wide_string_c2);
  EXPECT_TRUE(wide_string_c != wide_string_c3);
  EXPECT_TRUE(wide_string_c1 != wide_string_c);
  EXPECT_TRUE(wide_string_c2 != wide_string_c);
  EXPECT_TRUE(wide_string_c3 != wide_string_c);

  WideString wide_string_same1(L"hello");
  EXPECT_FALSE(wide_string_c != wide_string_same1);
  EXPECT_FALSE(wide_string_same1 != wide_string_c);

  WideString wide_string1(L"he");
  WideString wide_string2(L"hellp");
  WideString wide_string3(L"hellod");
  EXPECT_TRUE(wide_string_c != wide_string1);
  EXPECT_TRUE(wide_string_c != wide_string2);
  EXPECT_TRUE(wide_string_c != wide_string3);
  EXPECT_TRUE(wide_string1 != wide_string_c);
  EXPECT_TRUE(wide_string2 != wide_string_c);
  EXPECT_TRUE(wide_string3 != wide_string_c);

  const wchar_t* c_string_same1 = L"hello";
  EXPECT_FALSE(wide_string_c != c_string_same1);
  EXPECT_FALSE(c_string_same1 != wide_string_c);

  const wchar_t* c_string1 = L"he";
  const wchar_t* c_string2 = L"hellp";
  const wchar_t* c_string3 = L"hellod";
  EXPECT_TRUE(wide_string_c != c_string1);
  EXPECT_TRUE(wide_string_c != c_string2);
  EXPECT_TRUE(wide_string_c != c_string3);

  EXPECT_TRUE(c_string1 != wide_string_c);
  EXPECT_TRUE(c_string2 != wide_string_c);
  EXPECT_TRUE(c_string3 != wide_string_c);
}

TEST(WideStringView, Find) {
  WideStringView null_string;
  EXPECT_FALSE(null_string.Find(L'a').has_value());
  EXPECT_FALSE(null_string.Find(L'\0').has_value());

  WideStringView empty_string(L"");
  EXPECT_FALSE(empty_string.Find(L'a').has_value());
  EXPECT_FALSE(empty_string.Find(L'\0').has_value());

  pdfium::Optional<size_t> result;
  WideStringView single_string(L"a");
  result = single_string.Find(L'a');
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(0u, result.value());
  EXPECT_FALSE(single_string.Find(L'b').has_value());
  EXPECT_FALSE(single_string.Find(L'\0').has_value());

  WideStringView longer_string(L"abccc");
  result = longer_string.Find(L'a');
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(0u, result.value());
  result = longer_string.Find(L'c');
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(2u, result.value());
  EXPECT_FALSE(longer_string.Find(L'd').has_value());
  EXPECT_FALSE(longer_string.Find(L'\0').has_value());

  WideStringView hibyte_string(
      L"ab\xFF8c"
      L"def");
  result = hibyte_string.Find(L'\xFF8c');
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(2u, result.value());
}

TEST(WideStringView, NullIterator) {
  WideStringView null_str;
  int32_t sum = 0;
  bool any_present = false;
  for (const auto& c : null_str) {
    sum += c;  // Avoid unused arg warnings.
    any_present = true;
  }
  EXPECT_FALSE(any_present);
  EXPECT_EQ(0, sum);
}

TEST(WideStringView, EmptyIterator) {
  WideStringView empty_str(L"");
  int32_t sum = 0;
  bool any_present = false;
  for (const auto& c : empty_str) {
    any_present = true;
    sum += c;  // Avoid unused arg warnings.
  }
  EXPECT_FALSE(any_present);
  EXPECT_EQ(0, sum);
}

TEST(WideStringView, OneCharIterator) {
  WideStringView one_str(L"a");
  int32_t sum = 0;
  bool any_present = false;
  for (const auto& c : one_str) {
    any_present = true;
    sum += c;  // Avoid unused arg warnings.
  }
  EXPECT_TRUE(any_present);
  EXPECT_EQ(static_cast<int32_t>(L'a'), sum);
}

TEST(WideStringView, MultiCharIterator) {
  WideStringView one_str(L"abc");
  int32_t sum = 0;
  bool any_present = false;
  for (const auto& c : one_str) {
    any_present = true;
    sum += c;  // Avoid unused arg warnings.
  }
  EXPECT_TRUE(any_present);
  EXPECT_EQ(static_cast<int32_t>(L'a' + L'b' + L'c'), sum);
}

TEST(WideStringView, EmptyReverseIterator) {
  WideStringView empty;
  auto iter = empty.rbegin();
  EXPECT_TRUE(iter == empty.rend());
  EXPECT_FALSE(iter != empty.rend());
  EXPECT_FALSE(iter < empty.rend());
}

TEST(WideStringView, OneCharReverseIterator) {
  WideStringView one_str(L"a");
  auto iter = one_str.rbegin();
  EXPECT_FALSE(iter == one_str.rend());
  EXPECT_TRUE(iter != one_str.rend());
  EXPECT_TRUE(iter < one_str.rend());

  char ch = *iter++;
  EXPECT_EQ('a', ch);
  EXPECT_TRUE(iter == one_str.rend());
  EXPECT_FALSE(iter != one_str.rend());
  EXPECT_FALSE(iter < one_str.rend());
}

TEST(WideStringView, MultiCharReverseIterator) {
  WideStringView multi_str(L"abcd");
  auto iter = multi_str.rbegin();
  EXPECT_FALSE(iter == multi_str.rend());

  char ch = *iter++;
  EXPECT_EQ('d', ch);
  EXPECT_EQ('c', *iter);
  EXPECT_FALSE(iter == multi_str.rend());

  ch = *(++iter);
  EXPECT_EQ('b', ch);
  EXPECT_EQ('b', *iter);
  EXPECT_FALSE(iter == multi_str.rend());

  ch = *iter++;
  EXPECT_EQ('b', ch);
  EXPECT_EQ('a', *iter);
  EXPECT_FALSE(iter == multi_str.rend());

  ch = *iter++;
  EXPECT_EQ('a', ch);
  EXPECT_TRUE(iter == multi_str.rend());

  ch = *(--iter);
  EXPECT_EQ('a', ch);
  EXPECT_EQ('a', *iter);
  EXPECT_FALSE(iter == multi_str.rend());

  ch = *iter--;
  EXPECT_EQ('a', ch);
  EXPECT_EQ('b', *iter);
  EXPECT_FALSE(iter == multi_str.rend());

  ch = *iter--;
  EXPECT_EQ('b', ch);
  EXPECT_EQ('c', *iter);
  EXPECT_FALSE(iter == multi_str.rend());

  ch = *(--iter);
  EXPECT_EQ('d', ch);
  EXPECT_EQ('d', *iter);
  EXPECT_TRUE(iter == multi_str.rbegin());
}

TEST(WideStringView, AnyAllNoneOf) {
  WideStringView str(L"aaaaaaaaaaaaaaaaab");
  EXPECT_FALSE(std::all_of(str.begin(), str.end(),
                           [](const wchar_t& c) { return c == L'a'; }));

  EXPECT_FALSE(std::none_of(str.begin(), str.end(),
                            [](const wchar_t& c) { return c == L'a'; }));

  EXPECT_TRUE(std::any_of(str.begin(), str.end(),
                          [](const wchar_t& c) { return c == L'a'; }));

  EXPECT_TRUE(pdfium::ContainsValue(str, L'a'));
  EXPECT_TRUE(pdfium::ContainsValue(str, L'b'));
  EXPECT_FALSE(pdfium::ContainsValue(str, L'z'));
}

TEST(WideStringView, TrimmedRight) {
  WideStringView fred(L"FRED");
  EXPECT_EQ(L"FRED", fred.TrimmedRight(L'E'));
  EXPECT_EQ(L"FRE", fred.TrimmedRight(L'D'));
  WideStringView fredd(L"FREDD");
  EXPECT_EQ(L"FRE", fred.TrimmedRight(L'D'));
}

TEST(WideString, FormatWidth) {
  {
    WideString str;
    str.Format(L"%5d", 1);
    EXPECT_EQ(L"    1", str);
  }

  {
    WideString str;
    str.Format(L"%d", 1);
    EXPECT_EQ(L"1", str);
  }

  {
    WideString str;
    str.Format(L"%*d", 5, 1);
    EXPECT_EQ(L"    1", str);
  }

  {
    WideString str;
    str.Format(L"%-1d", 1);
    EXPECT_EQ(L"1", str);
  }

  {
    WideString str;
    str.Format(L"%0d", 1);
    EXPECT_EQ(L"1", str);
  }

  {
    WideString str;
    str.Format(L"%1048576d", 1);
    EXPECT_EQ(L"", str);
  }
}

TEST(WideString, FormatPrecision) {
  {
    WideString str;
    str.Format(L"%.2f", 1.12345);
    EXPECT_EQ(L"1.12", str);
  }

  {
    WideString str;
    str.Format(L"%.*f", 3, 1.12345);
    EXPECT_EQ(L"1.123", str);
  }

  {
    WideString str;
    str.Format(L"%f", 1.12345);
    EXPECT_EQ(L"1.123450", str);
  }

  {
    WideString str;
    str.Format(L"%-1f", 1.12345);
    EXPECT_EQ(L"1.123450", str);
  }

  {
    WideString str;
    str.Format(L"%0f", 1.12345);
    EXPECT_EQ(L"1.123450", str);
  }

  {
    WideString str;
    str.Format(L"%.1048576f", 1.2);
    EXPECT_EQ(L"", str);
  }
}

TEST(WideString, FormatOutOfRangeChar) {
  WideString str;
  str.Format(L"unsupported char '%c'", 0x00FF00FF);
}

TEST(WideString, Empty) {
  WideString empty_str;
  EXPECT_TRUE(empty_str.IsEmpty());
  EXPECT_EQ(0u, empty_str.GetLength());
  const wchar_t* cstr = empty_str.c_str();
  EXPECT_EQ(0u, FXSYS_wcslen(cstr));
}

TEST(CFX_WidString, InitializerList) {
  WideString many_str({L"clams", L" and ", L"oysters"});
  EXPECT_EQ(L"clams and oysters", many_str);
  many_str = {L"fish", L" and ", L"chips", L" and ", L"soda"};
  EXPECT_EQ(L"fish and chips and soda", many_str);
}

TEST(WideString, NullIterator) {
  WideString null_str;
  int32_t sum = 0;
  bool any_present = false;
  for (const auto& c : null_str) {
    sum += c;  // Avoid unused arg warnings.
    any_present = true;
  }
  EXPECT_FALSE(any_present);
  EXPECT_EQ(0, sum);
}

TEST(WideString, EmptyIterator) {
  WideString empty_str(L"");
  int32_t sum = 0;
  bool any_present = false;
  for (const auto& c : empty_str) {
    any_present = true;
    sum += c;  // Avoid unused arg warnings.
  }
  EXPECT_FALSE(any_present);
  EXPECT_EQ(0, sum);
}

TEST(WideString, OneCharIterator) {
  WideString one_str(L"a");
  int32_t sum = 0;
  bool any_present = false;
  for (const auto& c : one_str) {
    any_present = true;
    sum += c;  // Avoid unused arg warnings.
  }
  EXPECT_TRUE(any_present);
  EXPECT_EQ(static_cast<int32_t>(L'a'), sum);
}

TEST(WideString, MultiCharIterator) {
  WideString one_str(L"abc");
  int32_t sum = 0;
  bool any_present = false;
  for (const auto& c : one_str) {
    any_present = true;
    sum += c;  // Avoid unused arg warnings.
  }
  EXPECT_TRUE(any_present);
  EXPECT_EQ(static_cast<int32_t>(L'a' + L'b' + L'c'), sum);
}

TEST(WideString, AnyAllNoneOf) {
  WideString str(L"aaaaaaaaaaaaaaaaab");
  EXPECT_FALSE(std::all_of(str.begin(), str.end(),
                           [](const wchar_t& c) { return c == L'a'; }));

  EXPECT_FALSE(std::none_of(str.begin(), str.end(),
                            [](const wchar_t& c) { return c == L'a'; }));

  EXPECT_TRUE(std::any_of(str.begin(), str.end(),
                          [](const wchar_t& c) { return c == L'a'; }));

  EXPECT_TRUE(pdfium::ContainsValue(str, L'a'));
  EXPECT_TRUE(pdfium::ContainsValue(str, L'b'));
  EXPECT_FALSE(pdfium::ContainsValue(str, L'z'));
}

TEST(WideString, OStreamOverload) {
  std::ostringstream stream;

  // Basic case, empty string
  WideString str;
  stream << str;
  EXPECT_EQ("", stream.str());

  // Basic case, wide character
  str = L"\u20AC";
  stream << str;
  EXPECT_EQ("\u20AC", stream.str());

  // Basic case, non-empty string
  str = L"def";
  stream.str("");
  stream << "abc" << str << "ghi";
  EXPECT_EQ("abcdefghi", stream.str());

  // Changing the WideString does not change the stream it was written to.
  str = L"123";
  EXPECT_EQ("abcdefghi", stream.str());

  // Writing it again to the stream will use the latest value.
  stream.str("");
  stream << "abc" << str << "ghi";
  EXPECT_EQ("abc123ghi", stream.str());

  wchar_t stringWithNulls[]{'x', 'y', '\0', 'z'};

  // Writing a WideString with nulls and no specified length treats it as
  // a C-style null-terminated string.
  str = WideString(stringWithNulls);
  EXPECT_EQ(2u, str.GetLength());
  stream.str("");
  stream << str;
  EXPECT_EQ(2u, stream.tellp());

  // Writing a WideString with nulls but specifying its length treats it as
  // a C++-style string.
  str = WideString(stringWithNulls, 4);
  EXPECT_EQ(4u, str.GetLength());
  stream.str("");
  stream << str;
  EXPECT_EQ(4u, stream.tellp());

  // << operators can be chained.
  WideString str1(L"abc");
  WideString str2(L"def");
  stream.str("");
  stream << str1 << str2;
  EXPECT_EQ("abcdef", stream.str());
}

TEST(WideString, WideOStreamOverload) {
  std::wostringstream stream;

  // Basic case, empty string
  WideString str;
  stream << str;
  EXPECT_EQ(L"", stream.str());

  // Basic case, wide character
  str = L"\u20AC";
  stream << str;
  EXPECT_EQ(L"\u20AC", stream.str());

  // Basic case, non-empty string
  str = L"def";
  stream.str(L"");
  stream << L"abc" << str << L"ghi";
  EXPECT_EQ(L"abcdefghi", stream.str());

  // Changing the WideString does not change the stream it was written to.
  str = L"123";
  EXPECT_EQ(L"abcdefghi", stream.str());

  // Writing it again to the stream will use the latest value.
  stream.str(L"");
  stream << L"abc" << str << L"ghi";
  EXPECT_EQ(L"abc123ghi", stream.str());

  wchar_t stringWithNulls[]{'x', 'y', '\0', 'z'};

  // Writing a WideString with nulls and no specified length treats it as
  // a C-style null-terminated string.
  str = WideString(stringWithNulls);
  EXPECT_EQ(2u, str.GetLength());
  stream.str(L"");
  stream << str;
  EXPECT_EQ(2u, stream.tellp());

  // Writing a WideString with nulls but specifying its length treats it as
  // a C++-style string.
  str = WideString(stringWithNulls, 4);
  EXPECT_EQ(4u, str.GetLength());
  stream.str(L"");
  stream << str;
  EXPECT_EQ(4u, stream.tellp());

  // << operators can be chained.
  WideString str1(L"abc");
  WideString str2(L"def");
  stream.str(L"");
  stream << str1 << str2;
  EXPECT_EQ(L"abcdef", stream.str());
}

TEST(WideStringView, OStreamOverload) {
  // Basic case, empty string
  {
    std::ostringstream stream;
    WideStringView str;
    stream << str;
    EXPECT_EQ("", stream.str());
  }

  // Basic case, non-empty string
  {
    std::ostringstream stream;
    WideStringView str(L"def");
    stream << "abc" << str << "ghi";
    EXPECT_EQ("abcdefghi", stream.str());
  }

  // Basic case, wide character
  {
    std::ostringstream stream;
    WideStringView str(L"\u20AC");
    stream << str;
    EXPECT_EQ("\u20AC", stream.str());
  }

  // Changing the WideStringView does not change the stream it was written to.
  {
    std::ostringstream stream;
    WideStringView str(L"abc");
    stream << str;
    str = L"123";
    EXPECT_EQ("abc", stream.str());
  }

  // Writing it again to the stream will use the latest value.
  {
    std::ostringstream stream;
    WideStringView str(L"abc");
    stream << str;
    stream.str("");
    str = L"123";
    stream << str;
    EXPECT_EQ("123", stream.str());
  }

  // Writing a WideStringView with nulls and no specified length treats it as
  // a C-style null-terminated string.
  {
    wchar_t stringWithNulls[]{'x', 'y', '\0', 'z'};
    std::ostringstream stream;
    WideStringView str(stringWithNulls);
    EXPECT_EQ(2u, str.GetLength());
    stream << str;
    EXPECT_EQ(2u, stream.tellp());
    str = L"";
  }

  // Writing a WideStringView with nulls but specifying its length treats it as
  // a C++-style string.
  {
    wchar_t stringWithNulls[]{'x', 'y', '\0', 'z'};
    std::ostringstream stream;
    WideStringView str(stringWithNulls, 4);
    EXPECT_EQ(4u, str.GetLength());
    stream << str;
    EXPECT_EQ(4u, stream.tellp());
    str = L"";
  }

  // << operators can be chained.
  {
    std::ostringstream stream;
    WideStringView str1(L"abc");
    WideStringView str2(L"def");
    stream << str1 << str2;
    EXPECT_EQ("abcdef", stream.str());
  }
}

TEST(WideStringView, WideOStreamOverload) {
  // Basic case, empty string
  {
    std::wostringstream stream;
    WideStringView str;
    stream << str;
    EXPECT_EQ(L"", stream.str());
  }

  // Basic case, non-empty string
  {
    std::wostringstream stream;
    WideStringView str(L"def");
    stream << "abc" << str << "ghi";
    EXPECT_EQ(L"abcdefghi", stream.str());
  }

  // Basic case, wide character
  {
    std::wostringstream stream;
    WideStringView str(L"\u20AC");
    stream << str;
    EXPECT_EQ(L"\u20AC", stream.str());
  }

  // Changing the WideStringView does not change the stream it was written to.
  {
    std::wostringstream stream;
    WideStringView str(L"abc");
    stream << str;
    str = L"123";
    EXPECT_EQ(L"abc", stream.str());
  }

  // Writing it again to the stream will use the latest value.
  {
    std::wostringstream stream;
    WideStringView str(L"abc");
    stream << str;
    stream.str(L"");
    str = L"123";
    stream << str;
    EXPECT_EQ(L"123", stream.str());
  }

  // Writing a WideStringView with nulls and no specified length treats it as
  // a C-style null-terminated string.
  {
    wchar_t stringWithNulls[]{'x', 'y', '\0', 'z'};
    std::wostringstream stream;
    WideStringView str(stringWithNulls);
    EXPECT_EQ(2u, str.GetLength());
    stream << str;
    EXPECT_EQ(2u, stream.tellp());
  }

  // Writing a WideStringView with nulls but specifying its length treats it as
  // a C++-style string.
  {
    wchar_t stringWithNulls[]{'x', 'y', '\0', 'z'};
    std::wostringstream stream;
    WideStringView str(stringWithNulls, 4);
    EXPECT_EQ(4u, str.GetLength());
    stream << str;
    EXPECT_EQ(4u, stream.tellp());
  }

  // << operators can be chained.
  {
    std::wostringstream stream;
    WideStringView str1(L"abc");
    WideStringView str2(L"def");
    stream << str1 << str2;
    EXPECT_EQ(L"abcdef", stream.str());
  }
}

}  // namespace fxcrt
