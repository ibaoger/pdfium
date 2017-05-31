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

#define FPDF_ANNOT_UNKNOWN 0
#define FPDF_ANNOT_TEXT 1
#define FPDF_ANNOT_LINK 2
#define FPDF_ANNOT_FREETEXT 3
#define FPDF_ANNOT_LINE 4
#define FPDF_ANNOT_SQUARE 5
#define FPDF_ANNOT_CIRCLE 6
#define FPDF_ANNOT_POLYGON 7
#define FPDF_ANNOT_POLYLINE 8
#define FPDF_ANNOT_HIGHLIGHT 9
#define FPDF_ANNOT_UNDERLINE 10
#define FPDF_ANNOT_SQUIGGLY 11
#define FPDF_ANNOT_STRIKEOUT 12
#define FPDF_ANNOT_STAMP 13
#define FPDF_ANNOT_CARET 14
#define FPDF_ANNOT_INK 15
#define FPDF_ANNOT_POPUP 16
#define FPDF_ANNOT_FILEATTACHMENT 17
#define FPDF_ANNOT_SOUND 18
#define FPDF_ANNOT_MOVIE 19
#define FPDF_ANNOT_WIDGET 20
#define FPDF_ANNOT_SCREEN 21
#define FPDF_ANNOT_PRINTERMARK 22
#define FPDF_ANNOT_TRAPNET 23
#define FPDF_ANNOT_WATERMARK 24
#define FPDF_ANNOT_THREED 25
#define FPDF_ANNOT_RICHMEDIA 26
#define FPDF_ANNOT_XFAWIDGET 27

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
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFPage_GetAnnot(FPDF_PAGE page,
                                              int index,
                                              FPDF_ANNOTATION* annot);

// Get the subtype of an annotation.
//
//   annot  - handle to an annotation.
//
// Returns the annotation subtype. 0 if unknown.
DLLEXPORT FPDF_SUBTYPE STDCALL FPDFAnnotation_GetSubtype(FPDF_ANNOTATION annot);

// Get the color of an annotation. If no color is specified, default to yellow
// for highlight annotation, black for all else.
//
//   annot  - handle to an annotation.
//   R, G, B  - buffer to hold the RGB value of the color. Ranges from 0 to 255.
//   A      - buffer to hold the opacity. Ranges from 0 to 255.
//   type   - type of the color requested. Default to Color.
//
// Returns true if successful, false otherwise.
typedef enum _FPDFANNOT_COLORTYPE_ {
  Color = 0,
  InteriorColor
} FPDFANNOT_COLORTYPE;

DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnotation_GetColor(FPDF_ANNOTATION annot,
                        unsigned int* R,
                        unsigned int* G,
                        unsigned int* B,
                        unsigned int* A,
                        FPDFANNOT_COLORTYPE type = Color);

// Get the quadpoints of an annotation. Quadpoints are the vertices of the
// rectange that encompasses the texts affected by the annotation. Applicable to
// markup annotations only (highlight, strikeout, squiggly, underline, and
// link).
//
//   annot  - handle to an annotation.
//   quadPoints - receives the quadpoints
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnotation_GetQuadPoints(FPDF_ANNOTATION annot, FS_QUADPOINTSF* quadPoints);

// Get the annotation rectangle defining the location of the annotation.
//
//   annot  - handle to an annotation.
//   rect   - receives the annotation rectangle
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnotation_GetRect(FPDF_ANNOTATION annot,
                                                   FS_RECTF* rect);

// Get the contents of an annotation. |buffer| is only modified if |bufLen|
// is longer than the length of contents.
//
//   annot  - handle to an annotation.
//   buffer - buffer for holding the contents string, encoded in UTF8.
//   bufLen - length of the buffer.
//   type   - type of the text requested. Default to Contents.
//
// Returns the length of the contents.
typedef enum _FPDFANNOT_TEXTTYPE_ { Contents = 0, Author } FPDFANNOT_TEXTTYPE;
DLLEXPORT FPDF_DWORD STDCALL
FPDFAnnotation_GetText(FPDF_ANNOTATION annot,
                       char* buffer,
                       unsigned long bufLen,
                       FPDFANNOT_TEXTTYPE type = Contents);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // PUBLIC_FPDF_ANNOT_H_