// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PUBLIC_EXP_PDFIUM_H_
#define PUBLIC_EXP_PDFIUM_H_

#include <memory>

// NOLINTNEXTLINE(build/include)
#include "pdfium_config.h"
// NOLINTNEXTLINE(build/include)
#include "pdfium_document.h"
// NOLINTNEXTLINE(build/include)
#include "pdfium_status.h"
// NOLINTNEXTLINE(build/include)
#include "pdfium_unsupported_feature_delegate.h"

// This file is the main include for the PDFium API.

namespace pdfium {

void Initialize(std::unique_ptr<PDFiumConfig> cfg);
void SetUnsupportedFeatureDelegate(
    std::unique_ptr<UnsupportedFeatureDelegate> delgate);
void Shutdown();

}  // namespace pdfium

#endif  // PUBLIC_EXP_PDFIUM_H_
