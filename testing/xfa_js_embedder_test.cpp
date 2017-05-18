// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/xfa_js_embedder_test.h"

#include <string>

#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/parser/cxfa_scriptcontext.h"

XFAJSEmbedderTest::XFAJSEmbedderTest()
    : array_buffer_allocator_(pdfium::MakeUnique<FXJS_ArrayBufferAllocator>()) {
}

XFAJSEmbedderTest::~XFAJSEmbedderTest() {}

void XFAJSEmbedderTest::SetUp() {
  v8::Isolate::CreateParams params;
  params.array_buffer_allocator = array_buffer_allocator_.get();
  isolate_ = v8::Isolate::New(params);
  ASSERT_TRUE(isolate_ != nullptr);

  EmbedderTest::SetExternalIsolate(isolate_);
  EmbedderTest::SetUp();
}

void XFAJSEmbedderTest::TearDown() {
  value_ = nullptr;
  script_context_ = nullptr;

  EmbedderTest::TearDown();

  isolate_->Dispose();
  isolate_ = nullptr;
}

bool XFAJSEmbedderTest::OpenDocument(const std::string& filename,
                                     const char* password,
                                     bool must_linearize) {
  if (!EmbedderTest::OpenDocument(filename, password, must_linearize))
    return false;

  script_context_ = static_cast<CPDFXFA_Context*>(document())
                        ->GetXFADoc()
                        ->GetXFADoc()
                        ->GetScriptContext();
  return true;
}

bool XFAJSEmbedderTest::Execute(const CFX_ByteString& input) {
  value_ = pdfium::MakeUnique<CFXJSE_Value>(GetIsolate());
  if (script_context_->RunScript(
          XFA_SCRIPTLANGTYPE_Formcalc,
          CFX_WideString::FromUTF8(input.AsStringC()).AsStringC(), value_.get(),
          nullptr)) {
    return true;
  }

  CFXJSE_Value msg(GetIsolate());
  value_->GetObjectPropertyByIdx(1, &msg);
  EXPECT_TRUE(msg.IsString());

  printf("JS: %s\n", input.c_str());
  printf("JS ERROR: %ls\n", msg.ToWideString().c_str());
  return false;
}
