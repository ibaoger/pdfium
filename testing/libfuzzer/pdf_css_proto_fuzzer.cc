// Copyright 2017 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>

#include "core/fxcrt/cfx_seekablestreamproxy.h"
#include "core/fxcrt/css/cfx_css.h"
#include "core/fxcrt/css/cfx_csssyntaxparser.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"

#include "testing/libfuzzer/fuzzers/css.pb.h"
#include "testing/libfuzzer/fuzzers/css_proto_converter.h"
#include "third_party/libprotobuf-mutator/src/src/libfuzzer/libfuzzer_macro.h"

DEFINE_BINARY_PROTO_FUZZER(
    const css_proto_converter::StyleSheet& style_sheet_message) {
  static css_proto_converter::Converter* converter;
  static bool initialized = false;
  if (!initialized) {
    initialized = true;
    converter = new css_proto_converter::Converter();
  }

  std::string s = converter->Convert(style_sheet_message);
  auto input = WideString::FromUTF8(ByteStringView(s.c_str(), s.length()));

  CFX_CSSSyntaxParser parser(input.c_str(), input.GetLength());
  CFX_CSSSyntaxStatus status;
  do {
    status = parser.DoSyntaxParse();
  } while (status != CFX_CSSSyntaxStatus::Error &&
           status != CFX_CSSSyntaxStatus::EOS);
}
