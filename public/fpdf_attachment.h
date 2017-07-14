// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PUBLIC_FPDF_ATTACHMENT_H_
#define PUBLIC_FPDF_ATTACHMENT_H_

// NOLINTNEXTLINE(build/include)
#include "fpdfview.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef enum FPDFATTACHMENT_DATETYPE {
  FPDFATTACHMENT_DATETYPE_Creation = 0,
  FPDFATTACHMENT_DATETYPE_Modification,
} FPDFATTACHMENT_DATETYPE;

// Experimental API.
// Get the number of embedded files in |document|.
//
//   document - handle to a document.
//
// Returns the number of embedded files in |document|.
DLLEXPORT int STDCALL FPDFDoc_GetAttachmentCount(FPDF_DOCUMENT document);

// Experimental API.
// Get the name of the embedded file at |index| in |document|. |buffer| is
// only modified if |buflen| is longer than the length of the file name. On
// errors, |buffer| is unmodified and the returned length is 0.
//
//   document - handle to a document.
//   index    - the index of the requested embedded file.
//   buffer   - buffer for holding the file name, encoded in UTF16-LE.
//   buflen   - length of the buffer.
//
// Returns the length of the file name.
DLLEXPORT unsigned long STDCALL
FPDFDoc_GetAttachmentName(FPDF_DOCUMENT document,
                          int index,
                          void* buffer,
                          unsigned long buflen);

// Experimental API.
// Get the date associated with the embedded file at |index| in |document|.
// |buffer| is only modified if |buflen| is longer than the length of the file's
// date string. On errors, |buffer| is unmodified and the returned
// length is 0.
//
//   document - handle to a document.
//   index    - the index of the requested embedded file.
//   type     - type of the date requested.
//   buffer   - buffer for holding the file's date string, encoded in UTF16-LE.
//   buflen   - length of the buffer.
//
// Returns the length of the file's date string.
DLLEXPORT unsigned long STDCALL
FPDFDoc_GetAttachmentDate(FPDF_DOCUMENT document,
                          int index,
                          FPDFATTACHMENT_DATETYPE type,
                          void* buffer,
                          unsigned long buflen);

// Experimental API.
// Get the checksum string of the embedded file at |index| in |document|. If the
// checksum is a hexadecimal string, the returned string is the original string
// before decoding.  |buffer| is only modified if |buflen| is longer than the
// length of the file's checksum string. On errors, |buffer| is unmodified
// and the returned length is 0.
//
//   document - handle to a document.
//   index    - the index of the requested embedded file.
//   buffer   - buffer for holding the file's checksum, encoded in UTF16-LE.
//   buflen   - length of the buffer.
//
// Returns the length of the file's checksum string.
DLLEXPORT unsigned long STDCALL
FPDFDoc_GetAttachmentCheckSum(FPDF_DOCUMENT document,
                              int index,
                              void* buffer,
                              unsigned long buflen);

// Experimental API.
// Get the embedded file at |index| in |document|. |buffer| is only modified if
// |buflen| is longer than the length of the file. On errors, |buffer| is
// unmodified and the returned length is 0.
//
//   document - handle to a document.
//   index    - the index of the requested embedded file.
//   buffer   - buffer for holding the file's data in raw bytes.
//   buflen   - length of the buffer.
//
// Returns the length of the file.
DLLEXPORT unsigned long STDCALL
FPDFDoc_GetAttachmentFile(FPDF_DOCUMENT document,
                          int index,
                          void* buffer,
                          unsigned long buflen);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // PUBLIC_FPDF_ATTACHMENT_H_
