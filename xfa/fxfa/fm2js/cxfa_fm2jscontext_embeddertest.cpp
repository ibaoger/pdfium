// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/time.h>
#include <time.h>

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/xfa_js_embedder_test.h"

class FM2JSContextEmbedderTest : public XFAJSEmbedderTest {};

TEST_F(FM2JSContextEmbedderTest, TranslateEmpty) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  const char input[] = "";
  EXPECT_TRUE(Execute(input));
  // TODO(dsinclair): This should probably throw as a blank formcalc script
  // is invalid.
}

TEST_F(FM2JSContextEmbedderTest, TranslateNumber) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  const char input[] = "123";
  EXPECT_TRUE(Execute(input));

  CFXJSE_Value* value = GetValue();
  ASSERT_TRUE(value->IsInteger());
  EXPECT_EQ(123, value->ToInteger());
}

TEST_F(FM2JSContextEmbedderTest, Numeric) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    int result;
  } tests[] = {{"123 + 456", 579},
               {"2 - 3 * 10 / 2 + 7", -6},
               {"10 * 3 + 5 * 4", 50},
               {"(5 - \"abc\") * 3", 15},
               {"\"100\" / 10e1", 1},
               {"5 + null + 3", 8},
               {"if (\"abc\") then\n"
                "  10\n"
                "else\n"
                "  20\n"
                "endif",
                20},
               {"3 / 0 + 1", 0},
               {"-(17)", -17},
               {"-(-17)", 17},
               {"+(17)", 17},
               {"+(-17)", -17},
               {"if (1 < 2) then\n1\nendif", 1},
               {"if (\"abc\" > \"def\") then\n"
                "  1 and 0\n"
                "else\n"
                "  0\n"
                "endif",
                0}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsInteger());
    EXPECT_EQ(tests[i].result, value->ToInteger());
  }
}

TEST_F(FM2JSContextEmbedderTest, Strings) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {
      {"\"abc\"", "abc"},
      {"concat(\"The total is \", 2, \" dollars and \", 57, \"cents.\")",
       "The total is 2 dollars and 57 cents."}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Booleans) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    bool result;
  } tests[] = {{"0 and 1 or 2 > 1", true},
               {"2 < 3 not 1 == 1", false},
               {"\"abc\" | 2", true},
               {"1 or 0", true},
               {"0 | 0", false},
               {"0 or 1 | 0 or 0", true},
               {"1 and 0", false},
               {"0 & 0", true},
               {"0 and 1 & 0 and 0", false},
               {"not(\"true\")", true},
               {"not(1)", false},
               {"3 == 3", true},
               {"3 <> 4", true},
               {"\"abc\" eq \"def\"", false},
               {"\"def\" ne \"abc\"", true},
               {"5 + 5 == 10", true},
               {"5 + 5 <> \"10\"", false},
               {"3 < 3", false},
               {"3 > 4", false},
               {"\"abc\" <= \"def\"", true},
               {"\"def\" > \"abc\"", true},
               {"12 >= 12", true},
               {"\"true\" < \"false\"", false}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsBoolean());
    EXPECT_EQ(tests[i].result, value->ToBoolean());
  }
}

TEST_F(FM2JSContextEmbedderTest, Abs) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    float result;
  } tests[] = {{"Abs(1.03)", 1.03}, {"Abs(-1.03)", 1.03}, {"Abs(0)", 0}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsNumber());
    EXPECT_EQ(tests[i].result, value->ToFloat());
  }
}

TEST_F(FM2JSContextEmbedderTest, Avg) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    float result;
  } tests[] = {{"Avg(0, 32, 16)", 16}, {"Avg(2.5, 17, null)", 9.75}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsNumber());
    EXPECT_EQ(tests[i].result, value->ToFloat());
  }
}

TEST_F(FM2JSContextEmbedderTest, Ceil) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    int result;
  } tests[] = {{"Ceil(2.5875)", 3}, {"Ceil(-5.9)", -5}, {"Ceil(\"abc\")", 0}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsInteger());
    EXPECT_EQ(tests[i].result, value->ToInteger());
  }
}

TEST_F(FM2JSContextEmbedderTest, Count) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    int result;
  } tests[] = {{"Count(\"Tony\", \"Blue\", 41)", 3}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsInteger());
    EXPECT_EQ(tests[i].result, value->ToInteger());
  }
}

TEST_F(FM2JSContextEmbedderTest, Floor) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    int result;
  } tests[] = {{"Floor(21.3409873)", 21},
               {"Floor(5.999965342)", 5},
               {"Floor(3.2 * 15)", 48}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsInteger());
    EXPECT_EQ(tests[i].result, value->ToInteger());
  }
}

TEST_F(FM2JSContextEmbedderTest, Max) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    int result;
  } tests[] = {{"Max(234, 15, 107)", 234},
               {"Max(\"abc\", 15, \"Tony Blue\")", 15},
               {"Max(\"abc\")", 0}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsInteger());
    EXPECT_EQ(tests[i].result, value->ToInteger());
  }
}

TEST_F(FM2JSContextEmbedderTest, Min) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    int result;
  } tests[] = {{"Min(234, 15, 107)", 15},
               {"Min(\"abc\", 15, \"Tony Blue\")", 15},
               {"Min(\"abc\")", 0}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsInteger());
    EXPECT_EQ(tests[i].result, value->ToInteger());
  }
}

TEST_F(FM2JSContextEmbedderTest, Mod) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    int result;
  } tests[] = {{"Mod(64, -3)", 1}, {"Mod(-13, 3)", -1}, {"Mod(\"abc\", 2)", 0}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsInteger());
    EXPECT_EQ(tests[i].result, value->ToInteger());
  }
}

TEST_F(FM2JSContextEmbedderTest, Round) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    float result;
  } tests[] = {{"Round(12.389764537, 4)", 12.3898},
               {"Round(20/3, 2)", 6.67},
               {"Round(8.9897, \"abc\")", 9},
               {"Round(FV(400, 0.10/12, 30*12), 2)", 904195.17}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsNumber());
    EXPECT_EQ(tests[i].result, value->ToDouble());
  }
}

TEST_F(FM2JSContextEmbedderTest, Sum) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    int result;
  } tests[] = {{"Sum(2, 4, 6, 8)", 20},
               {"Sum(-2, 4, -6, 8)", 4},
               {"Sum(4, 16, \"abc\", 19)", 39}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsInteger());
    EXPECT_EQ(tests[i].result, value->ToInteger());
  }
}

TEST_F(FM2JSContextEmbedderTest, Date) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  time_t seconds = time(nullptr);
  int days = seconds / (60 * 60 * 24);

  EXPECT_TRUE(Execute("Date()"));

  CFXJSE_Value* value = GetValue();
  ASSERT_TRUE(value->IsNumber());
  EXPECT_EQ(days, value->ToInteger());
}

TEST_F(FM2JSContextEmbedderTest, Date2Num) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    int result;
  } tests[] = {
      {"Date2Num(\"Mar 15, 1996\")", 35138},
      {"Date2Num(\"1/1/1900\", \"D/M/YYYY\")", 1},
      {"Date2Num(\"03/15/96\", \"MM/DD/YY\")", 35138},
      {"Date2Num(\"Aug 1, 1996\", \"MMM D, YYYY\")", 35277},
      {"Date2Num(\"96-08-20\", \"YY-MM-DD\", \"fr_FR\")", 35296},
      {"Date2Num(\"1/3/00\", \"D/M/YY\") - Date2Num(\"1/2/00\", \"D/M/YY\")",
       29}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsInteger());
    EXPECT_EQ(tests[i].result, value->ToInteger());
  }
}

TEST_F(FM2JSContextEmbedderTest, DateFmt) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"DateFmt(1)", "M/D/YY"},
               {"DateFmt(2, \"fr_CA\")", "YY-MM-DD"},
               {"DateFmt(3, \"de_DE\")", "D. MMMM YYYY"},
               {"DateFmt(4, \"fr_FR\")", "EEE D' MMMM YYYY"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, IsoDate2Num) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    int result;
  } tests[] = {{"IsoDate2Num(\"1900\")", 1},
               {"IsoDate2Num(\"1900-01\")", 1},
               {"IsoDate2Num(\"1900-01-01\")", 1},
               {"IsoDate2Num(\"19960315T20:20:20\")", 35138},
               {"IsoDate2Num(\"2000-03-01\") - IsoDate2Num(\"20000201\")", 29}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsInteger());
    EXPECT_EQ(tests[i].result, value->ToInteger());
  }
}

TEST_F(FM2JSContextEmbedderTest, IsoTime2Num) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    int result;
  } tests[] = {{"IsoTime2Num(\"00:00:00Z\")", 1}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsInteger());
    EXPECT_EQ(tests[i].result, value->ToInteger());
  }
}

TEST_F(FM2JSContextEmbedderTest, LocalDateFmt) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"LocalDateFmt(1, \"de_DE\")", "tt.MM.uu"},
               {"LocalDateFmt(2, \"fr_CA\")", "aa-MM-jj"},
               {"LocalDateFmt(3, \"de_CH\")", "t. MMMM jjjj"},
               {"LocalDateFmt(4, \"fr_FR\")", "EEEE j MMMM aaaa"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, LocalTimeFmt) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"LocalTimeFmt(1, \"de_DE\")", "HH:mm"},
               {"LocalTimeFmt(2, \"fr_CA\")", "HH:mm::ss"},
               {"LocalTimeFmt(3, \"de_CH\")", "HH:mm:ss z"},
               {"LocalTimeFmt(4, \"fr_FR\")", "HH' h 'mm z"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Num2Date) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Num2Date(1, \"DD/MM/YYYY\")", "01/01/1900"},
               {"Num2Date(35139, \"DD-MMM-YYYY\", \"de_DE\")", "16-Mrz-1996"},
               {"Num2Date(Date2Num(\"Mar 15, 2000\") - Date2Num(\"98-03-15\", "
                "\"YY-MM-DD\", \"fr_CA\"))",
                "Jan 1, 1902"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Num2GMTime) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Num2GMTime(1, \"HH:MM:SS\")", "00:00:00"},
               {"Num2GMTime(65593001, \"HH:MM:SS Z\")", "18:13:13 GMT"},
               {"Num2GMTime(43993001, TimeFmt(4, \"de_DE\"), \"de_DE\")",
                "12.13 Uhr GMT"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Num2Time) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Num2Time(1, \"HH:MM:SS\")", "00:00:00"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Time) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));
  struct timeval tp;
  gettimeofday(&tp, nullptr);

  EXPECT_TRUE(Execute("Time()"));

  CFXJSE_Value* value = GetValue();
  ASSERT_TRUE(value->IsInteger());
  EXPECT_EQ(tp.tv_sec * 1000L + tp.tv_usec / 1000, value->ToInteger());
}

TEST_F(FM2JSContextEmbedderTest, Time2Num) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    int result;
  } tests[] = {
      {"Time2Num(\"00:00:00 GMT\", \"HH:MM:SS Z\")", 1},
      {"Time2Num(\"13:13:13 GMT\", \"HH:MM:SS Z\", \"fr_FR\")", 47593001}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsInteger());
    EXPECT_EQ(tests[i].result, value->ToInteger());
  }
}

TEST_F(FM2JSContextEmbedderTest, TimeFmt) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"TimeFmt(1)", "h::MM A"},
               {"TimeFmt(2, \"fr_CA\")", "HH:MM:SS"},
               {"TimeFmt(3, \"fr_FR\")", "HH:MM:SS Z"},
               {"TimeFmt(4, \"de_DE\")", "H.MM' Uhr 'Z"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Apr) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    float result;
  } tests[] = {{"Apr(35000, 269.50, 360)", 0.08515404566},
               {"Apr(210000 * 0.75, 850 + 110, 25 * 26)", 0.07161332404}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsNumber());
    EXPECT_EQ(tests[i].result, value->ToDouble());
  }
}

TEST_F(FM2JSContextEmbedderTest, CTerm) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    float result;
  } tests[] = {
      {"CTerm(0.02, 1000, 100)", 116.2767474515},
      {"CTerm(0.10, 500000, 12000)", 39.13224648502},
      {"CTerm(0.0275 + 0.0025, 1000000, 55000 * 0.10)", 176.02226044975}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsNumber());
    EXPECT_EQ(tests[i].result, value->ToDouble());
  }
}

TEST_F(FM2JSContextEmbedderTest, FV) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    float result;
  } tests[] = {{"FV(400, 0.10 / 12, 30 * 12)", 904195.16991842445},
               {"FV(1000, 0.075 / 4, 10 * 4)", 58791.96145535981}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsNumber());
    EXPECT_EQ(tests[i].result, value->ToDouble());
  }
}

TEST_F(FM2JSContextEmbedderTest, IPmt) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    float result;
  } tests[] = {{"IPmt(30000, 0.085, 295.50, 7, 3)", 624.8839283142},
               {"IPmt(160000, 0.0475, 980, 24, 12)", 7103.80833569485},
               {"IPmt(15000, 0.065, 65.50, 15, 1)", 0}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsNumber());
    EXPECT_EQ(tests[i].result, value->ToDouble());
  }
}

TEST_F(FM2JSContextEmbedderTest, NPV) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    float result;
  } tests[] = {{"NPV(0.065, 5000)", 4694.83568075117},
               {"NPV(0.10, 500, 1500, 4000, 10000)", 11529.60863329007},
               {"NPV(0.0275 / 12, 50, 60, 40, 100, 25)", 273.14193838457}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsNumber());
    EXPECT_EQ(tests[i].result, value->ToDouble());
  }
}

TEST_F(FM2JSContextEmbedderTest, Pmt) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    float result;
  } tests[] = {{"Pmt(150000, 0.0475 / 12, 25 * 12)", 855.17604207164},
               {"Pmt(25000, 0.085, 12)", 3403.82145169876}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsNumber());
    EXPECT_EQ(tests[i].result, value->ToDouble());
  }
}

TEST_F(FM2JSContextEmbedderTest, PPmt) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    float result;
  } tests[] = {{"PPmt(30000, 0.085, 295.50, 7, 3)", 261.6160716858},
               {"PPmt(160000, 0.0475, 980, 24, 12)", 4656.19166430515},
               {"PPmt(15000, 0.065, 65.50, 15, 1)", 0}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsNumber());
    EXPECT_EQ(tests[i].result, value->ToDouble());
  }
}

TEST_F(FM2JSContextEmbedderTest, PV) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    float result;
  } tests[] = {{"PV(400, 0.10 / 12, 30 * 12)", 45580.32799074439},
               {"PV(1000, 0.075 / 4, 10 * 4)", 58791.96145535981}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsNumber());
    EXPECT_EQ(tests[i].result, value->ToDouble());
  }
}

TEST_F(FM2JSContextEmbedderTest, Rate) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    float result;
  } tests[] = {{"Rate(12000, 8000, 5)", 0.0844717712},
               {"Rate(10000, 0.25 * 5000, 4 * 12)", 0.04427378243}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsNumber());
    EXPECT_EQ(tests[i].result, value->ToDouble());
  }
}

TEST_F(FM2JSContextEmbedderTest, Term) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    float result;
  } tests[] = {{"Term(475, .05, 1500)", 3.00477517728},
               {"Term(2500, 0.0275 + 0.0025, 5000)", 1.97128786369}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsNumber());
    EXPECT_EQ(tests[i].result, value->ToDouble());
  }
}

TEST_F(FM2JSContextEmbedderTest, Choose) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  EXPECT_TRUE(
      Execute("Choose(3, \"Taxes\", \"Price\", \"Person\", \"Teller\")"));
  CFXJSE_Value* value = GetValue();
  ASSERT_TRUE(value->IsString());
  EXPECT_EQ("Person", value->ToString());

  EXPECT_TRUE(Execute("Choose(2, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)"));
  value = GetValue();
  ASSERT_TRUE(value->IsInteger());
  EXPECT_EQ(9, value->ToInteger());

  EXPECT_TRUE(Execute(
      "Choose(20/3, \"A\", \"B\", \"C\", \"D\", \"E\", \"F\", \"G\", \"H\")"));
  value = GetValue();
  ASSERT_TRUE(value->IsString());
  EXPECT_EQ("F", value->ToString());
}

TEST_F(FM2JSContextEmbedderTest, Exists) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  EXPECT_TRUE(Execute("Exists(\"hello world\")"));
  CFXJSE_Value* value = GetValue();
  ASSERT_TRUE(value->IsBoolean());
  EXPECT_EQ(false, value->ToBoolean());
}

TEST_F(FM2JSContextEmbedderTest, HasValue) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    bool result;
  } tests[] = {{"HasValue(2)", true}, {"HasValue(\" \")", false}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsBoolean());
    EXPECT_EQ(tests[i].result, value->ToBoolean());
  }
}

TEST_F(FM2JSContextEmbedderTest, Oneof) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    bool result;
  } tests[] = {
      {"Oneof(3, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)", true},
      {"Oneof(\"John\", \"Bill\", \"Gary\", \"Joan\", \"John\", \"Lisa\")",
       true},
      {"Oneof(3, 1, 25)", false}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsBoolean());
    EXPECT_EQ(tests[i].result, value->ToBoolean());
  }
}

TEST_F(FM2JSContextEmbedderTest, Within) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    bool result;
  } tests[] = {{"Within(\"C\", \"A\", \"D\")", true},
               {"Within(1.5, 0, 2)", true},
               {"Within(-1, 0, 2)", false}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsBoolean());
    EXPECT_EQ(tests[i].result, value->ToBoolean());
  }
}

TEST_F(FM2JSContextEmbedderTest, Eval) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    int result;
  } tests[] = {{"eval(\"10*3+5*4\")", 50}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsInteger());
    EXPECT_EQ(tests[i].result, value->ToInteger());
  }
}

TEST_F(FM2JSContextEmbedderTest, Null) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Null()", "null"},
               {"Concat(\"ABC\", Null(), \"DEF\")", "ABCDEF"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }

  EXPECT_TRUE(Execute("Null() + 5"));

  CFXJSE_Value* value = GetValue();
  ASSERT_TRUE(value->IsInteger());
  EXPECT_EQ(5, value->ToInteger());
}

TEST_F(FM2JSContextEmbedderTest, Ref) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Ref(\"10*3+5*4\")", "10*3+5*4"}, {"Ref(\"hello\")", "hello"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, UnitType) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"UnitType(\"36 in\")", "in"},
               {"UnitType(\"2.54centimeters\")", "cm"},
               {"UnitType(\"picas\")", "pt"},
               {"UnitType(\"2.cm\")", "cm"},
               {"UnitType(\"2.zero cm\")", "in"},
               {"UnitType(\"kilometers\")", "in"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, UnitValue) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    float result;
  } tests[] = {{"UnitValue(\"2in\")", 2},
               {"UnitValue(\"2in\", \"cm\")", 5.08},
               {"UnitValue(\"6\", \"pt\")", 432},
               {"UnitType(\"A\", \"cm\")", 0},
               {"UnitType(\"5.08cm\", \"kilograms\")", 2}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsNumber());
    EXPECT_EQ(tests[i].result, value->ToFloat());
  }
}

TEST_F(FM2JSContextEmbedderTest, At) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    int result;
  } tests[] = {{"At(\"ABCDEFGH\", \"AB\")", 1},
               {"At(\"ABCDEFGH\", \"F\")", 6},
               {"At(23412931298471, 29)", 5}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsInteger());
    EXPECT_EQ(tests[i].result, value->ToInteger());
  }
}

TEST_F(FM2JSContextEmbedderTest, Concat) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Concat(\"ABC\", \"DEF\")", "ABCDEF"},
               {"Concat(\"Tony\", Space(1), \"Blue\")", "Tony Blue"},
               {"Concat(\"You owe \", WordNum(1154.67, 2), \".\")",
                "You owe One Thousand One Hundred Fifty-four Dollars And "
                "Sixty-seven Cents."}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Decode) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {
      {"Decode(\"&AElig;&Aacute;&Acirc;&Aacute;&Acirc;\", \"html\")", "ÆÁÂÁÂ"},
      {"Decode(\"~!@#$%%^&amp;*()_+|`{&quot;}[]&lt;&gt;?,./;&apos;:\", "
       "\"xml\")",
       "~!@#$%%^&*()_+|`{"
       "}[]<>?,./;':"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Encode) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {
      {"Encode(\"\"\"hello, world!\"\"\", \"url\")",
       "%%22hello,%%20world!%%22"},
      {"Encode(\"ÁÂÃÄÅÆ\", \"html\")", "&#xc1;&#Xc2;&#Xc3;&#xc4;&#xc5;&#xc6;"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Format) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Format(\"MMM D, YYYY\", \"20020901\")", "Sep 1, 2002"},
               {"Format(\"$9,999,999.99\", 1234567.89)", "$1,234,567.89"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Left) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Left(\"ABCDEFGH\", 3)", "ABC"},
               {"Left(\"Tony Blue\", 5)", "Tony "}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Len) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    int result;
  } tests[] = {
      {"Len(\"ABCDEFGH\")", 8}, {"Len(4)", 1}, {"Len(Str(4.532, 6, 4))", 6}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsInteger());
    EXPECT_EQ(tests[i].result, value->ToInteger());
  }
}

TEST_F(FM2JSContextEmbedderTest, Lower) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Lower(\"ABC\")", "abc"},
               {"Lower(\"21 Main St.\")", "21 main st."},
               {"Lower(15)", "15"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Ltrim) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Ltrim(\"   ABCD\")", "ABCD"},
               {"Ltrim(Rtrim(\"    Tony Blue    \"))", "Tony Blue"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Parse) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Parse(\"MMM D, YYYY\", \"Sep 1, 2002\")", "2002-09-01"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }

  EXPECT_TRUE(Execute("Parse(\"$9,999,999.99\", \"$1,234,567.89\")"));
  CFXJSE_Value* value = GetValue();
  ASSERT_TRUE(value->IsNumber());
  EXPECT_EQ(1234567.89, value->ToFloat());
}

TEST_F(FM2JSContextEmbedderTest, Replace) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Replace(\"Tony Blue\", \"Tony\", \"Chris\")", "Chris Blue"},
               {"Replace(\"ABCDEFGH\", \"D\")", "ABCEFGH"},
               {"Replace(\"ABCDEFGH\", \"d\")", "ABCDEFGH"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Right) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Right(\"ABCDEFGH\", 3)", "FGH"},
               {"Right(\"Tony Blue\", 5)", " Blue"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Rtrim) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Rtrim(\"ABCD   \")", "ABCD"},
               {"Rtrim(\"Tony Blue      \t\")", "Tony Blue"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Space) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Space(5)", "     "},
               {"Concat(\"Tony\", Space(1), \"Blue\")", "Tony Blue"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Str) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Str(2.456)", "         2"},
               {"Str(4.532, 6, 4)", "4.5320"},
               {"Str(234.458, 4)", " 234"},
               {"Str(31.2345, 4, 2)", "****"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Stuff) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Stuff(\"TonyBlue\", 5, 0, \" \")", "Tony Blue"},
               {"Stuff(\"ABCDEFGH\", 4, 2)", "ABCFGH"},
               {"Stuff(\"members-list@myweb.com\", 0, 0, \"cc:\")",
                "cc:members-list@myweb.com"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Substr) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Substr(\"ABCDEFG\", 3, 4)", "CDEF"},
               {"Substr(3214, 2, 1)", "2"},
               {"Substr(\"ABCDEFG\", 5, 0)", ""},
               {"Substr(\"21 Waterloo St.\", 4, 5)", "Water"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Uuid) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  EXPECT_TRUE(Execute("Uuid()"));

  CFXJSE_Value* value = GetValue();
  ASSERT_TRUE(value->IsString());
}

TEST_F(FM2JSContextEmbedderTest, Upper) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {{"Upper(\"abc\")", "ABC"},
               {"Upper(\"21 Main St.\")", "21 MAIN ST."},
               {"Upper(15)", "15"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, WordNum) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  struct {
    const char* program;
    const char* result;
  } tests[] = {
      {"WordNum(123.45)", "One Hundred and Twenty-three Dollars"},
      {"WordNum(123.45, 1)", "One Hundred and Twenty-three Dollars"},
      {"WordNum(1154.67, 2)",
       "One Thousand One Hundred Fifty-four Dollars And Sixty-seven Cents"},
      {"WordNum(43, 2)", "Forty-three Dollars And Zero Cents"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(Execute(tests[i].program));

    CFXJSE_Value* value = GetValue();
    ASSERT_TRUE(value->IsString());
    EXPECT_EQ(tests[i].result, value->ToString());
  }
}

TEST_F(FM2JSContextEmbedderTest, Get) {
  // TODO(dsinclair): Is this supported?
}

TEST_F(FM2JSContextEmbedderTest, Post) {
  // TODO(dsinclair): Is this supported?
}

TEST_F(FM2JSContextEmbedderTest, Put) {
  // TODO(dsinclair): Is this supported?
}
