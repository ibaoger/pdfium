// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef PUBLIC_EXP_PDFIUM_H_
#define PUBLIC_EXP_PDFIUM_H_

/**
 * This file defines the generic PDFium interfaces.
 */

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

enum pdfium_status_t {
  PDFIUM_STATUS_SUCCESS = 0,
  PDFIUM_STATUS_UNKNOWN = 1,
  PDFIUM_STATUS_FILE_ERROR = 2,
  PDFIUM_STATUS_FORMAT_ERROR = 3,
  PDFIUM_STATUS_PASSWORD_ERROR = 4,
  PDFIUM_STATUS_SECURITY_ERROR = 5,
  PDFIUM_STATUS_PAGE_NOT_FOUND_OR_CONTENT_ERROR = 6,

#ifdef PDF_ENABLE_XFA
  PDFIUM_STATUS_XFA_LOAD_ERROR = 7,
  PDFIUM_STATUS_XFA_LAYOUT_ERROR = 8,
#endif  // PDF_ENABLE_XFA
};

struct pdfium_config_t {
  uint32_t version;

  const char** user_font_paths;

  void* js_isolate;
  unsigned int js_embedder_slot;
};

/**
 * Initialize the PDFium library. Must be called before any other PDFium
 * methods are used.
 */
DLLEXPORT void STDCALL pdfium_initialize(pdfium_config_t*);

/**
 * Shutdown the PDFium library. Must not call any PDFium methods after this
 * method returns.
 */
DLLEXPORT void STDCALL pdfium_shutdown();

/**
 * Retrieve the last status error. Returns |pdfium_status_t| representing the
 * status of the last method call. See |pdfium_error_str| to convert the error
 * code into a string.
 */
DLLEXPORT pdfium_status_t STDCALL pdfium_last_status();

/**
 * Retrieves the string representation of the |pdfium_status_t|. The string
 * is owned by PDFium and should not be freed. The returned string is ASCII.
 */
DLLEXPORT const char* STDCALL pdfium_error_str(pdfium_status_t);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // PUBLIC_EXP_PDFIUM_H_
