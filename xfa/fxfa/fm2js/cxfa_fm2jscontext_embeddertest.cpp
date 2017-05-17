// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"
#include "fxjs/fxjs_v8.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/parser/cxfa_scriptcontext.h"

class FM2JSContextEmbedderTest : public EmbedderTest {
 public:
  FM2JSContextEmbedderTest()
      : array_buffer_allocator_(
            pdfium::MakeUnique<FXJS_ArrayBufferAllocator>()) {}

  void SetUp() override {
    v8::Isolate::CreateParams params;
    params.array_buffer_allocator = array_buffer_allocator_.get();
    isolate_ = v8::Isolate::New(params);
    ASSERT(isolate_);

    EmbedderTest::SetExternalIsolate(isolate_);
    EmbedderTest::SetUp();

    ASSERT(OpenDocument("simple_xfa.pdf"));

    script_context_ = static_cast<CPDFXFA_Context*>(document())
                          ->GetXFADoc()
                          ->GetXFADoc()
                          ->GetScriptContext();
  }

  void TearDown() override {
    value_ = nullptr;
    script_context_ = nullptr;

    EmbedderTest::TearDown();

    isolate_->Dispose();
    isolate_ = nullptr;
  }

  v8::Isolate* GetIsolate() const { return isolate_; }

  bool Execute(const CFX_ByteString& input) {
    value_ = pdfium::MakeUnique<CFXJSE_Value>(GetIsolate());
    if (script_context_->RunScript(
            XFA_SCRIPTLANGTYPE_Formcalc,
            CFX_WideString::FromUTF8(input.AsStringC()).AsStringC(),
            value_.get(), nullptr)) {
      return true;
    }

    CFXJSE_Value msg(GetIsolate());
    value_->GetObjectPropertyByIdx(1, &msg);
    ASSERT(msg.IsString());

    printf("JS: %s\n", input.c_str());
    printf("JS ERROR: %ls\n", msg.ToWideString().c_str());
    return false;
  }

  CFXJSE_Value* GetValue() const { return value_.get(); }

 private:
  std::unique_ptr<FXJS_ArrayBufferAllocator> array_buffer_allocator_;
  std::unique_ptr<CFXJSE_Value> value_;
  v8::Isolate* isolate_;
  CXFA_ScriptContext* script_context_;
};

TEST_F(FM2JSContextEmbedderTest, TranslateEmpty) {
  const wchar_t input[] = L"";
  EXPECT_TRUE(Execute(FX_UTF8Encode(input)));
  // TODO(dsinclair): This should probably throw as a blank formcalc script
  // is invalid.
}

TEST_F(FM2JSContextEmbedderTest, TranslateNumber) {
  const wchar_t input[] = L"123";
  EXPECT_TRUE(Execute(FX_UTF8Encode(input)));

  CFXJSE_Value* value = GetValue();
  ASSERT_TRUE(value->IsNumber());
  EXPECT_EQ(123, value->ToInteger());
}

TEST_F(FM2JSContextEmbedderTest, Add) {
  const wchar_t input[] = L"123 + 456";
  EXPECT_TRUE(Execute(FX_UTF8Encode(input)));

  CFXJSE_Value* value = GetValue();
  ASSERT_TRUE(value->IsNumber());
  EXPECT_EQ(579, value->ToInteger());
}
