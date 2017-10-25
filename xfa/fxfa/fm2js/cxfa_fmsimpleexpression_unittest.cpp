// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/fm2js/cxfa_fmsimpleexpression.h"

#include <memory>
#include <utility>

#include "core/fxcrt/cfx_widetextbuf.h"
#include "core/fxcrt/fx_string.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/fm2js/cxfa_fmlexer.h"
#include "xfa/fxfa/fm2js/cxfa_fmtojavascriptdepth.h"

TEST(FMCallExpressionTest, more_than_32_arguments) {
  // Use sign as it has 3 object parameters at positions 0, 5, and 6.
  auto exp = pdfium::MakeUnique<CXFA_FMIdentifierExpression>(0, L"sign");

  std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> args;
  for (size_t i = 0; i < 50; i++)
    args.push_back(pdfium::MakeUnique<CXFA_FMSimpleExpression>(0, TOKnan));

  CXFA_FMToJavaScriptDepth::Reset();
  CXFA_FMCallExpression callExp(0, std::move(exp), std::move(args), true);
  CFX_WideTextBuf js;
  callExp.ToJavaScript(js);

  // Generate the result javascript string.
  WideString result = L"sign(";
  for (size_t i = 0; i < 50; i++) {
    if (i > 0)
      result += L", ";

    result += L"pfm_rt.get_";
    // Object positions for sign() method.
    if (i == 0 || i == 5 || i == 6)
      result += L"jsobj()";
    else
      result += L"val()";
  }
  result += L")";

  EXPECT_EQ(result.AsStringView(), js.AsStringView());
}

TEST(FMStringExpressionTest, Empty) {
  CXFA_FMToJavaScriptDepth::Reset();
  CFX_WideTextBuf accumulator;
  CXFA_FMStringExpression(1, WideStringView()).ToJavaScript(accumulator);
  EXPECT_EQ(L"", accumulator.AsStringView());
}

TEST(FMStringExpressionTest, Short) {
  CXFA_FMToJavaScriptDepth::Reset();
  CFX_WideTextBuf accumulator;
  CXFA_FMStringExpression(1, L"a").ToJavaScript(accumulator);
  EXPECT_EQ(L"a", accumulator.AsStringView());
}

TEST(FMStringExpressionTest, Medium) {
  CXFA_FMToJavaScriptDepth::Reset();
  CFX_WideTextBuf accumulator;
  CXFA_FMStringExpression(1, L".abcd.").ToJavaScript(accumulator);
  EXPECT_EQ(L"\"abcd\"", accumulator.AsStringView());
}

TEST(FMStringExpressionTest, Long) {
  CXFA_FMToJavaScriptDepth::Reset();
  CFX_WideTextBuf accumulator;
  std::vector<WideStringView::UnsignedType> vec(140000, L'A');
  CXFA_FMStringExpression(1, WideStringView(vec)).ToJavaScript(accumulator);
  EXPECT_EQ(140000u, accumulator.GetLength());
}

TEST(FMStringExpressionTest, Quoted) {
  CXFA_FMToJavaScriptDepth::Reset();
  CFX_WideTextBuf accumulator;
  CXFA_FMStringExpression(1, L".Simon says \"\"run\"\".")
      .ToJavaScript(accumulator);
  EXPECT_EQ(L"\"Simon says \\\"run\\\"\"", accumulator.AsStringView());
}
