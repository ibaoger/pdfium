// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_pagelabel.h"

#include <array>
#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfdoc/cpdf_numbertree.h"
#include "core/fxcrt/stl_util.h"

namespace {

WideString MakeRoman(int num) {
  const auto kArabic = fxcrt::ToArray<const int>(
      {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1});
  const auto kRoman = fxcrt::ToArray<const WideStringView>(
      {L"m", L"cm", L"d", L"cd", L"c", L"xc", L"l", L"xl", L"x", L"ix", L"v",
       L"iv", L"i"});
  const int kMaxNum = 1000000;

  num %= kMaxNum;
  int i = 0;
  WideString result;
  while (num > 0) {
    while (num >= kArabic[i]) {
      num = num - kArabic[i];
      result += kRoman[i];
    }
    i = i + 1;
  }
  return result;
}

WideString MakeLetters(int num) {
  if (num == 0) {
    return WideString();
  }

  const int kMaxCount = 1000;
  const int kLetterCount = 26;
  --num;

  int count = num / kLetterCount + 1;
  count %= kMaxCount;
  wchar_t ch = L'a' + num % kLetterCount;
  WideString result;
  for (int i = 0; i < count; i++) {
    result += ch;
  }
  return result;
}

WideString GetLabelNumPortion(int num, const ByteString& style) {
  if (style.IsEmpty()) {
    return WideString();
  }
  if (style == "D") {
    return WideString::FormatInteger(num);
  }
  if (style == "R") {
    WideString result = MakeRoman(num);
    result.MakeUpper();
    return result;
  }
  if (style == "r") {
    return MakeRoman(num);
  }
  if (style == "A") {
    WideString result = MakeLetters(num);
    result.MakeUpper();
    return result;
  }
  if (style == "a") {
    return MakeLetters(num);
  }
  return WideString();
}

}  // namespace

CPDF_PageLabel::CPDF_PageLabel(CPDF_Document* doc) : doc_(doc) {}

CPDF_PageLabel::~CPDF_PageLabel() = default;

std::optional<WideString> CPDF_PageLabel::GetLabel(int page_index) const {
  if (!doc_) {
    return std::nullopt;
  }

  if (page_index < 0 || page_index >= doc_->GetPageCount()) {
    return std::nullopt;
  }

  const CPDF_Dictionary* root_dict = doc_->GetRoot();
  if (!root_dict) {
    return std::nullopt;
  }

  RetainPtr<const CPDF_Dictionary> labels_dict =
      root_dict->GetDictFor("PageLabels");
  if (!labels_dict) {
    return std::nullopt;
  }

  CPDF_NumberTree number_tree(std::move(labels_dict));
  RetainPtr<const CPDF_Object> label_value;
  int n = page_index;
  while (n >= 0) {
    label_value = number_tree.LookupValue(n);
    if (label_value) {
      break;
    }
    n--;
  }

  if (label_value) {
    label_value = label_value->GetDirect();
    if (const CPDF_Dictionary* label_dict = label_value->AsDictionary()) {
      WideString label;
      if (label_dict->KeyExist("P")) {
        label += label_dict->GetUnicodeTextFor("P");
      }

      ByteString style = label_dict->GetByteStringFor("S", ByteString());
      int label_number = page_index - n + label_dict->GetIntegerFor("St", 1);
      label += GetLabelNumPortion(label_number, style);
      return label;
    }
  }
  return WideString::FormatInteger(page_index + 1);
}
