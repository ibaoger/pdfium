// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_BARCODETP_H_
#define XFA_FWL_THEME_CFWL_BARCODETP_H_

#include "fxjs/gc/heap.h"
#include "xfa/fwl/theme/cfwl_utils.h"
#include "xfa/fwl/theme/cfwl_widgettp.h"

namespace pdfium {

class CFWL_BarcodeTP final : public CFWL_WidgetTP {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_BarcodeTP() override;

  // CFWL_WidgetTP
  void DrawBackground(const CFWL_ThemeBackground& pParams) override;

 private:
  CFWL_BarcodeTP();
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_BarcodeTP;

#endif  // XFA_FWL_THEME_CFWL_BARCODETP_H_
