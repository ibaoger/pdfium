// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_MEMORY_H_
#define CORE_FXCRT_FX_MEMORY_H_

#include "core/fxcrt/fx_system.h"

#ifdef __cplusplus
#include <stdlib.h>
#include <limits>
#include <memory>
#include <new>

#include "third_party/base/allocator/partition_allocator/partition_alloc.h"

extern pdfium::base::PartitionAllocatorGeneric gArrayBufferPartitionAllocator;
extern pdfium::base::PartitionAllocatorGeneric gStringPartitionAllocator;

NEVER_INLINE void FX_OutOfMemoryTerminate();

inline void* FX_SafeRealloc(void* ptr, size_t num_members, size_t member_size) {
  if (num_members < std::numeric_limits<size_t>::max() / member_size) {
    return realloc(ptr, num_members * member_size);
  }
  return nullptr;
}

inline void* FX_AllocOrDie(size_t num_members, size_t member_size) {
  // TODO(tsepez): See if we can avoid the implicit memset(0).
  if (void* result = calloc(num_members, member_size)) {
    return result;
  }
  FX_OutOfMemoryTerminate();  // Never returns.
  return nullptr;             // Suppress compiler warning.
}

inline void* FX_AllocOrDie2D(size_t w, size_t h, size_t member_size) {
  if (w < std::numeric_limits<size_t>::max() / h) {
    return FX_AllocOrDie(w * h, member_size);
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

// Never returns nullptr.
#define FX_Alloc(type, size) (type*)FX_AllocOrDie(size, sizeof(type))
#define FX_Alloc2D(type, w, h) (type*)FX_AllocOrDie2D(w, h, sizeof(type))
#define FX_Realloc(type, ptr, size) \
  (type*)FX_ReallocOrDie(ptr, size, sizeof(type))

// May return nullptr.
#define FX_TryAlloc(type, size) (type*)calloc(size, sizeof(type))
#define FX_TryRealloc(type, ptr, size) \
  (type*)FX_SafeRealloc(ptr, size, sizeof(type))

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

// Used with std::unique_ptr to free raw memory.
struct FxFreeDeleter {
  inline void operator()(void* ptr) const { free(ptr); }
};

// Used with std::unique_ptr to Release() objects that can't be deleted.
template <class T>
struct ReleaseDeleter {
  inline void operator()(T* ptr) const { ptr->Release(); }
};

#endif  // __cplusplus

#endif  // CORE_FXCRT_FX_MEMORY_H_
