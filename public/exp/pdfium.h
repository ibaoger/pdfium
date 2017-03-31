// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef PUBLIC_EXP_PDFIUM_H_
#define PUBLIC_EXP_PDFIUM_H_

#include <stdint.h>

#if defined(_WIN32) && defined(FPDFSDK_EXPORTS)
// On Windows system, functions are exported in a DLL
#define DLLEXPORT __declspec(dllexport)
#define STDCALL __stdcall
#else
#define DLLEXPORT
#define STDCALL
#endif

#include "public/exp/document.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pdfium_config_t {
  uint32_t version;

  const char** user_font_paths;

  void* js_isolate;
  unsigned int js_embedder_slot;
};

DLLEXPORT void STDCALL pdfium_initialize(pdfium_config_t* config);

DLLEXPORT void STDCALL pdfium_shutdown();

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // PUBLIC_EXP_PDFIUM_H_
