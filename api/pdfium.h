// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef API_PDFIUM_H_
#define API_PDFIUM_H_

#include <memory>
#include <utility>
#include <vector>

#include "public/fpdf_ext.h"

class CCodec_ModuleMgr;
class CFX_GEModule;
class CPDF_ModuleMgr;

namespace pdfium {

class PDFium;
class PDFiumConfig;
class UnsupportedFeatureDelegate;

struct PDFIUM_UNSUPPORT_INFO : public UNSUPPORT_INFO {
  PDFium* parent_;
};

class PDFium {
 public:
  explicit PDFium(std::unique_ptr<PDFiumConfig> cfg);
  ~PDFium();

  void SetUnsupportedFeatureDelegate(
      std::unique_ptr<UnsupportedFeatureDelegate> delegate) {
    unsupported_feature_delegate_ = std::move(delegate);
  }
  UnsupportedFeatureDelegate* GetUnsupportedFeatureDelegate() const {
    return unsupported_feature_delegate_.get();
  }

 private:
  PDFium(const PDFium&) = delete;
  PDFium(PDFium&&) = delete;
  PDFium operator=(const PDFium&) = delete;

  std::unique_ptr<PDFiumConfig> cfg_;
  std::unique_ptr<CCodec_ModuleMgr> codec_module_mgr_;
  CFX_GEModule* font_mgr_;
  CPDF_ModuleMgr* module_mgr_;
  std::vector<const char*> font_paths_;  // Pointers into cfg->font_paths;
  std::unique_ptr<UnsupportedFeatureDelegate> unsupported_feature_delegate_;

  PDFIUM_UNSUPPORT_INFO unsupport_info_;
};

}  // namespace pdfium

#endif  // API_PDFIUM_H_
