// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <stdlib.h>  // For abort().

#include "core/fxcrt/fx_memory.h"

pdfium::base::PartitionAllocatorGeneric gArrayBufferPartitionAllocator;
pdfium::base::PartitionAllocatorGeneric gStringPartitionAllocator;

NEVER_INLINE void FX_OutOfMemoryTerminate() {
  // Termimate cleanly if we can, else crash at a specific address (0xbd).
  abort();
#ifndef _WIN32
  reinterpret_cast<void (*)()>(0xbd)();
#endif
}
