// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef PUBLIC_FPDF_ANNOT_H_
#define PUBLIC_FPDF_ANNOT_H_

// NOLINTNEXTLINE(build/include)
#include "fpdfview.h"

#include "public/fpdf_doc.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Get the number of annotations in |page|.
//
//   page   - handle to a page.
//
// Returns the number of annotations in |page|.
DLLEXPORT int STDCALL FPDFPage_GetAnnotCount(FPDF_PAGE page);

// Get annotation in |page| at |index|.
//
//   page  - handle to a page.
//   index - the index of the annotation.
//   annot - receives the annotation
//
// Returns true if successful, false if failed.
DLLEXPORT FPDF_BOOL STDCALL FPDFPage_GetAnnot(FPDF_PAGE page,
                                              int index,
                                              void** annot);

// Get the subtype of an annotation.
//
//   annot  - handle to an annotation.
//   buffer - buffer for holding the subtype string.
//   bufLen - length of the buffer.
//
// Returns true if successful, false if failed.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnotation_GetSubtype(FPDF_ANNOTATION annot,
                                                      char* buffer,
                                                      unsigned long bufLen);

// Get the color of an annotation. If no color is specified, default to yellow
// for highlight annotation, black for all else.
//
//   annot  - handle to an annotation.
//   buffer - buffer for holding the color string.
//   bufLen - length of the buffer.
//
// The color string is of one of the below 3 formats:
//   RGB colors:        R G B RG (for stroke), R G B rg (for fill)
//   Grayscale colors:  Gray G (for stroke), Gray g (for fill)
//   CMYK colors:       C M Y K K (for stroke), C M Y K k (for fill)
//
// Returns true if successful, false if failed.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnotation_GetColor(FPDF_ANNOTATION annot,
                                                    char* buffer,
                                                    unsigned long bufLen);

// Get the quadpoints of an annotation. Applicable to: highlight, strikeout,
// squiggly, underline, and link.
//
//   annot  - handle to an annotation.
//   quadPoints - receives the quadpoints
//
// Returns true if successful, false if failed.
DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnotation_GetQuadPoints(FPDF_ANNOTATION annot, FS_QUADPOINTSF* quadPoints);

// Get the rectangle for an annotation.
//
//   annot  - handle to an annotation.
//   rect   - receives the annotation rectangle
//
// Returns true if successful, false if failed.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnotation_GetRect(FPDF_ANNOTATION annot,
                                                   FS_RECTF* rect);

// Get the contents of an annotation.
//
//   annot  - handle to an annotation.
//   buffer - buffer for holding the contents string.
//   bufLen - length of the buffer.
//
// Returns true if successful, false if failed.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnotation_GetContents(FPDF_ANNOTATION annot,
                                                       char* buffer,
                                                       unsigned long bufLen);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // PUBLIC_FPDF_ANNOT_H_