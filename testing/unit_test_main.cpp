// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "core/fxcrt/fx_memory.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

#if PDF_ENABLE_XFA
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fgas/font/cfgas_gefont.h"

namespace {

// The loading time of the CFGAS_FontMgr is linear in the number of times it is
// loaded. So, if a test suite has a lot of tests that need a font manager they
// can end up executing very, very slowly.
class Environment : public testing::Environment {
 public:
  void SetUp() override {
    // TODO(dsinclair): This font loading is slow. We should make a test font
    // loader which loads up a single font we use in all tests.
    CFX_GEModule::Get()->GetFontMgr()->SetSystemFontInfo(
        IFX_SystemFontInfo::CreateDefault(nullptr));

    font_mgr_ = pdfium::MakeUnique<CFGAS_FontMgr>();
    if (!font_mgr_->EnumFonts())
      font_mgr_ = nullptr;
  }

  void TearDown() override {
    font_mgr_.reset();
  }
  CFGAS_FontMgr* FontManager() const { return font_mgr_.get(); }

 private:
  std::unique_ptr<CFGAS_FontMgr> font_mgr_;
};

Environment* env_ = nullptr;

}  // namespace

CFGAS_FontMgr* GetGlobalFontManager() {
  return env_->FontManager();
}
#endif  // PDF_ENABLE_XFA

// Can't use gtest-provided main since we need to initialize partition
// alloc before invoking any test.
int main(int argc, char** argv) {
  FXMEM_InitializePartitionAlloc();

#if PDF_ENABLE_XFA
  env_ = new Environment();
  // The env will be deleted by gtest.
  AddGlobalTestEnvironment(env_);
#endif  // PDF_ENABLE_XFA

  testing::InitGoogleTest(&argc, argv);
  testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
