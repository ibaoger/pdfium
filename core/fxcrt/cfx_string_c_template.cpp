// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/cfx_string_c_template.h"

#include "core/fxcrt/fx_string.h"

std::ostream& operator<<(std::ostream& os, const CFX_ByteStringC& str) {
  return os.write(str.c_str(), str.GetLength());
}

std::wostream& operator<<(std::wostream& os, const CFX_WideStringC& str) {
  return os.write(str.c_str(), str.GetLength());
}

std::ostream& operator<<(std::ostream& os, const CFX_WideStringC& str) {
  os << FX_UTF8Encode(str);
  return os;
}
