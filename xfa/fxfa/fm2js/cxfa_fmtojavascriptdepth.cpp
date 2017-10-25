// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/fm2js/cxfa_fmtojavascriptdepth.h"

namespace {

const unsigned int kMaxDepth = 1250;

}  // namespace

unsigned long CXFA_FMToJavaScriptDepth::depth_ = 0;
unsigned long CXFA_FMToJavaScriptDepth::max_depth_ = kMaxDepth;

void CXFA_FMToJavaScriptDepth::Reset() {
  depth_ = 0;
}

bool CXFA_FMToJavaScriptDepth::IncrementAndCheck() {
  return ++depth_ < max_depth_;
}
