// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_MEMORY_H_
#define CORE_FXCRT_FX_MEMORY_H_

#include "core/fxcrt/fx_system.h"

#ifdef __cplusplus
extern "C" {
#endif

// For external C libraries to malloc through PDFium. These may return nullptr.
void* FXMEM_DefaultAlloc(size_t byte_size, int flags);
void* FXMEM_DefaultRealloc(void* pointer, size_t new_size, int flags);
void FXMEM_DefaultFree(void* pointer, int flags);

#ifdef __cplusplus
}  // extern "C"

#include <stdlib.h>
#include <limits>
#include <memory>
#include <new>

#include "third_party/base/allocator/partition_allocator/partition_alloc.h"
#include "third_party/base/numerics/safe_math.h"

using SafeSize = pdfium::base::CheckedNumeric<size_t>;

extern pdfium::base::PartitionAllocatorGeneric gArrayBufferPartitionAllocator;
extern pdfium::base::PartitionAllocatorGeneric gGeneralPartitionAllocator;
extern pdfium::base::PartitionAllocatorGeneric gStringPartitionAllocator;

NEVER_INLINE void FX_OutOfMemoryTerminate();

inline void* FX_SafeAlloc(size_t count, size_t size) {
  SafeSize total = size;
  total *= count;
  if (!total.IsValid()) {
    return nullptr;
  }
  return pdfium::base::PartitionAllocGeneric(gGeneralPartitionAllocator.root(),
                                             total.ValueOrDie(),
                                             "GeneralPartition");
}

inline void* FX_SafeRealloc(void* ptr, size_t num_members, size_t member_size) {
  SafeSize size = num_members;
  size *= member_size;
  if (!size.IsValid()) {
    return nullptr;
  }
  return pdfium::base::PartitionReallocGeneric(
      gGeneralPartitionAllocator.root(), ptr, size.ValueOrDie(),
      "GeneralPartition");
}

inline void* FX_AllocOrDie(size_t num_members, size_t member_size) {
  void* result = FX_SafeAlloc(num_members, member_size);
  if (!result) {
    FX_OutOfMemoryTerminate();
  }
  return result;
}

inline void* FX_AllocOrDie2D(size_t w, size_t h, size_t member_size) {
  SafeSize size = w;
  size *= h;
  if (size.IsValid()) {
    return FX_AllocOrDie(size.ValueOrDie(), member_size);
  }
  FX_OutOfMemoryTerminate();  // Never returns.
  return nullptr;             // Suppress compiler warning.
}

inline void* FX_ReallocOrDie(void* ptr,
                             size_t num_members,
                             size_t member_size) {
  if (void* result = FX_SafeRealloc(ptr, num_members, member_size)) {
    return result;
  }
  FX_OutOfMemoryTerminate();  // Never returns.
  return nullptr;             // Suppress compiler warning.
}

// These never return nullptr.
#define FX_Alloc(type, size) (type*)FX_AllocOrDie(size, sizeof(type))
#define FX_Alloc2D(type, w, h) (type*)FX_AllocOrDie2D(w, h, sizeof(type))
#define FX_Realloc(type, ptr, size) \
  (type*)FX_ReallocOrDie(ptr, size, sizeof(type))

// These may return nullptr.
#define FX_TryAlloc(type, count) (type*)FX_SafeAlloc(count, sizeof(type))
#define FX_TryRealloc(type, ptr, count) \
  (type*)FX_SafeRealloc(ptr, count, sizeof(type))

#define FX_Free(ptr) pdfium::base::PartitionFree(ptr)

// The FX_ArraySize(arr) macro returns the # of elements in an array arr.
// The expression is a compile-time constant, and therefore can be
// used in defining new arrays, for example.  If you use FX_ArraySize on
// a pointer by mistake, you will get a compile-time error.
//
// One caveat is that FX_ArraySize() doesn't accept any array of an
// anonymous type or a type defined inside a function.
#define FX_ArraySize(array) (sizeof(ArraySizeHelper(array)))

// This template function declaration is used in defining FX_ArraySize.
// Note that the function doesn't need an implementation, as we only
// use its type.
template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];

// Used with std::unique_ptr to FX_Free raw memory.
struct FxFreeDeleter {
  inline void operator()(void* ptr) const { FX_Free(ptr); }
};

// Used with std::unique_ptr to Release() objects that can't be deleted.
template <class T>
struct ReleaseDeleter {
  inline void operator()(T* ptr) const { ptr->Release(); }
};

#endif  // __cplusplus

#endif  // CORE_FXCRT_FX_MEMORY_H_
