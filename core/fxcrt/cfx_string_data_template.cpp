// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/cfx_string_data_template.h"
#include "third_party/base/allocator/partition_allocator/partition_alloc.h"

pdfium::base::PartitionAllocatorGeneric g_partitionAllocator;
bool g_partitionAllocatorInitialized = false;
