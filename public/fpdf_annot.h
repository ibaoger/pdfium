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

typedef enum FPDFANNOT_COLORTYPE {
  FPDFANNOT_COLORTYPE_Color = 0,
  FPDFANNOT_COLORTYPE_InteriorColor
} FPDFANNOT_COLORTYPE;

typedef enum FPDFANNOT_TEXTTYPE {
  FPDFANNOT_TEXTTYPE_Contents = 0,
  FPDFANNOT_TEXTTYPE_Author
} FPDFANNOT_TEXTTYPE;

// Check if an annotation subtype is currently supported for creating and
// displaying.
//
//   subtype   - the subtype to be checked.
//
// Returns true if this subtype supported, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_IsSupportedSubtype(FPDF_ANNOTATION_SUBTYPE subtype);

// Create an annotation in |page| of the subtype |subtype|. If the specified
// subtype is illegal or unsupported, then a new annotation will not be created.
//
//   page      - handle to a page.
//   subtype   - the subtype of the new annotation.
//   annot     - receives the newly created annotation.
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL
FPDFPage_CreateAnnot(FPDF_PAGE page,
                     FPDF_ANNOTATION_SUBTYPE subtype,
                     FPDF_ANNOTATION* annot);

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
//   annot - receives the annotation.
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFPage_GetAnnot(FPDF_PAGE page,
                                              int index,
                                              FPDF_ANNOTATION* annot);

// Get the subtype of an annotation.
//
//   annot  - handle to an annotation.
//
// Returns the annotation subtype.
DLLEXPORT FPDF_ANNOTATION_SUBTYPE STDCALL
FPDFAnnot_GetSubtype(FPDF_ANNOTATION annot);

// Check if the annotation has its appearance stream defined.
//
//    annot  - handle to an annotation.
//
// Returns true if the annation has its AP stream defined, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_HasAPStream(FPDF_ANNOTATION annot);

// Get the form object for parsing an annotation's appearance stream. A form
// object is needed to retrieve the paths, resources, etc. defined by the
// annotation's appearance stream. Must call FPDFAnnot_CloseForm() on the form
// after it is no longer needed.
//
//    annot  -  handle to an annotation.
//    page   -  handle to the page that the annotation is on.
//    form   -  receives the form.
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_GetForm(FPDF_ANNOTATION annot,
                                              FPDF_PAGE page,
                                              FPDF_FORM* form);

// Close a form object returned by FPDFAnnot_GetForm().
//
//    form   - handle to a form.
DLLEXPORT void STDCALL FPDFAnnot_CloseForm(FPDF_FORM form);

// Set the |annot|'s appearance stream to be the path as defined by |path|.
// Since |annot| will not take ownership of |path|, |path| can be destroyed
// after a call to this method; see FPDFPageObj_DestroyPath().
//
//    annot     -  handle to an annotation.
//    document  -  handle to the document that this annotation is on; only
//                 needed if the annotation does not have its appearance stream
//                 defined yet.
//    page      -  handle to the page that the annotation is on.
//    path      -  handle to the path that the annotation will be set to have.
//
// Return true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetPathObject(FPDF_ANNOTATION annot,
                                                    FPDF_DOCUMENT document,
                                                    FPDF_PAGE page,
                                                    FPDF_PAGEOBJECT path);

// Get the number of path objects in |form|.
//
//    form   - handle to a form.
//
// Returns the number of path objects in |form|.
DLLEXPORT int STDCALL FPDFForm_GetPathObjectCount(FPDF_FORM form);

// Get the path object in |form| at |index|.
//
//  form   - handle to a form.
//  index  - the index of the path object.
//  path   - receives the path.
//
// Return true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFForm_GetPathObject(FPDF_FORM form,
                                                   int index,
                                                   FPDF_PAGEOBJECT* path);

// Set the color of an annotation. Fails when called on annotations with
// appearance streams already defined; use instead
// FPDFPath_Set{Stroke|Fill}Color().
//
//   annot    - handle to an annotation.
//   type     - type of the color to be set.
//   R, G, B  - buffer to hold the RGB value of the color. Ranges from 0 to 255.
//   A        - buffer to hold the opacity. Ranges from 0 to 255.
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetColor(FPDF_ANNOTATION annot,
                                               FPDFANNOT_COLORTYPE type,
                                               unsigned int R,
                                               unsigned int G,
                                               unsigned int B,
                                               unsigned int A);

// Get the color of an annotation. If no color is specified, default to yellow
// for highlight annotation, black for all else. Fails when called on
// annotations with appearance streams already defined; use instead
// FPDFPath_GetFillColor().
//
//   annot    - handle to an annotation.
//   type     - type of the color requested.
//   R, G, B  - buffer to hold the RGB value of the color. Ranges from 0 to 255.
//   A        - buffer to hold the opacity. Ranges from 0 to 255.
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_GetColor(FPDF_ANNOTATION annot,
                                               FPDFANNOT_COLORTYPE type,
                                               unsigned int* R,
                                               unsigned int* G,
                                               unsigned int* B,
                                               unsigned int* A);

// Check if the annotation is of a type that has attachment points
// (i.e. quadpoints). Quadpoints are the vertices of the rectange that
// encompasses the texts affected by the annotation. They provide the
// coordinates in the page where the annotation is attached. Only text markup
// annotations (i.e. highlight, strikeout, squiggly, and underline) and link
// annotations have quadpoints.
//
//   annot  - handle to an annotation.
//
// Returns true if the annotation is of a type that has quadpoints, false
// otherwise.
DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_HasAttachmentPoints(FPDF_ANNOTATION annot);

// Set the attachment points (i.e. quadpoints) of an annotation. If the
// annotation's appearance stream is defined and this annotation is of a type
// with quadpoints, then update the bounding box too.
//
//   annot      - handle to an annotation.
//   quadPoints - the quadpoints to be set.
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_SetAttachmentPoints(FPDF_ANNOTATION annot, FS_QUADPOINTSF quadPoints);

// Get the attachment points (i.e. quadpoints) of an annotation. If the
// annotation's appearance stream is defined and this annotation is of a type
// with quadpoints, then return the bounding box it specifies instead.
//
//   annot      - handle to an annotation.
//   quadPoints - receives the attachment points.
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_GetAttachmentPoints(FPDF_ANNOTATION annot,
                              FS_QUADPOINTSF* quadPoints);

// Set the annotation rectangle defining the location of the annotation. If the
// annotation's appearance stream is defined and this annotation is of a type
// without quadpoints, then update the bounding box too.
//
//   annot  - handle to an annotation.
//   rect   - the annotation rectangle to be set.
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetRect(FPDF_ANNOTATION annot,
                                              FS_RECTF rect);

// Get the annotation rectangle defining the location of the annotation. If the
// annotation's appearance stream is defined and this annotation is of a type
// without quadpoints, then return the bounding box it specifies instead.
//
//   annot  - handle to an annotation.
//   rect   - receives the annotation rectangle.
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_GetRect(FPDF_ANNOTATION annot,
                                              FS_RECTF* rect);

// Set the contents of an annotation.
//
//   annot  - handle to an annotation.
//   type   - type of the text to be set.
//   text   - the text to be set.
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetText(FPDF_ANNOTATION annot,
                                              FPDFANNOT_TEXTTYPE type,
                                              FPDF_WIDESTRING text);

// Get the contents of an annotation. |buffer| is only modified if |buflen|
// is longer than the length of contents.
//
//   annot  - handle to an annotation.
//   type   - type of the text requested.
//   buffer - buffer for holding the contents string, encoded in UTF16-LE.
//   buflen - length of the buffer.
//
// Returns the length of the contents.
DLLEXPORT unsigned long STDCALL FPDFAnnot_GetText(FPDF_ANNOTATION annot,
                                                  FPDFANNOT_TEXTTYPE type,
                                                  void* buffer,
                                                  unsigned long buflen);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // PUBLIC_FPDF_ANNOT_H_
