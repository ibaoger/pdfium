// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "api/pdfium.h"

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxge/cfx_gemodule.h"
#include "fpdfsdk/javascript/ijs_runtime.h"
#include "public/exp/pdfium.h"
#include "third_party/base/allocator/partition_allocator/partition_alloc.h"
#include "third_party/base/ptr_util.h"

#ifdef PDF_ENABLE_XFA
#include "fxbarcode/BC_Library.h"
#endif  // PDF_ENABLE_XFA

namespace pdfium {
namespace {

PDFium* pdfium_ = nullptr;

// TODO(dsinclair): Redo the unsupported handlers to accept the delegate
// directly instead of going through the C callback.
extern "C" void UnsupportedFeatureHandler(UNSUPPORT_INFO* info, int type) {
  static_cast<PDFIUM_UNSUPPORT_INFO*>(info)
      ->parent_->GetUnsupportedFeatureDelegate()
      ->HandleUnsupportedFeature(
          static_cast<UnsupportedFeatureDelegate::Type>(type));
}

}  // namespace

PDFium::PDFium(std::unique_ptr<PDFiumConfig> cfg)
    : cfg_(std::move(cfg)),
      unsupported_feature_delegate_(MakeUnique<UnsupportedFeatureDelegate>()) {
  codec_module_mgr_ = MakeUnique<CCodec_ModuleMgr>();

  for (const auto& path : cfg_->font_paths)
    font_paths_.push_back(path.c_str());

  // TODO(dsinclair): Make these not singletons when the old API is removed.
  // Then font_mgr_ and module_mgr_ turn into unique_ptrs.
  font_mgr_ = CFX_GEModule::Get();
  font_mgr_->Init(font_paths_.data(), codec_module_mgr_.get());

  module_mgr_ = CPDF_ModuleMgr::Get();
  module_mgr_->SetCodecModule(codec_module_mgr_.get());
  module_mgr_->InitPageModule();
  module_mgr_->LoadEmbeddedMaps();
  module_mgr_->LoadCodecModules();

#ifdef PDF_ENABLE_XFA
  FXJSE_Initialize();
  BC_Library_Init();
#endif  // PDF_ENABLE_XFA

  IJS_Runtime::Initialize(cfg_->embedder_slot, cfg_->isolate);

  memset(&unsupport_info_, 0, sizeof(PDFIUM_UNSUPPORT_INFO));
  unsupport_info_.version = 1;
  unsupport_info_.FSDK_UnSupport_Handler = UnsupportedFeatureHandler;
  unsupport_info_.parent_ = this;

  FSDK_SetUnSpObjProcessHandler(&unsupport_info_);
}

PDFium::~PDFium() {
#ifdef PDF_ENABLE_XFA
  BC_Library_Destory();
  FXJSE_Finalize();
#endif  // PDF_ENABLE_XFA

  CPDF_ModuleMgr::Destroy();
  CFX_GEModule::Destroy();
  codec_module_mgr_.reset();
  IJS_Runtime::Destroy();
}

void Initialize(std::unique_ptr<PDFiumConfig> cfg) {
  if (pdfium_)
    return;

  FXMEM_InitalizePartitionAlloc();
  pdfium_ = new PDFium(std::move(cfg));
}

void SetUnsupportedFeatureDelegate(
    std::unique_ptr<UnsupportedFeatureDelegate> delegate) {
  if (!pdfium_)
    return;
  pdfium_->SetUnsupportedFeatureDelegate(std::move(delegate));
}

void Shutdown() {
  delete pdfium_;
  pdfium_ = nullptr;
}

}  // namespace pdfium
