// Copyright 2016 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "core/fxcrt/fx_string.h"
#include "xfa/fwl/cfx_barcode.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < 8)
    return 0;

  BC_TYPE type = static_cast<BC_TYPE>(data[0] % (BC_DATAMATRIX + 1));
  data += 4;
  size -= 4;

  CFX_Barcode barcode;
  if (!barcode.Create(type))
    return 0;

  // TODO(tsepez): Setup more options from |data|.
  barcode.SetModuleHeight(300);
  barcode.SetModuleWidth(420);
  barcode.SetHeight(298);
  barcode.SetWidth(418);

  CFX_WideStringC content(reinterpret_cast<const wchar_t*>(data),
                          size / sizeof(wchar_t));

  if (!barcode.Encode(content, false))
    return 0;

  // TODO(tsepez): Output to device.
  return 0;
}
