// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/exp/pdfium.h"

namespace pdfium {

PDFiumConfig::PDFiumConfig() : embedder_slot(0), isolate(nullptr) {}

PDFiumConfig::~PDFiumConfig() {}

}  // namespace pdfium
