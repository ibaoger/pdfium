// Copyright 2016 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include "public/fpdf_edit.h"
#include "public/fpdf_flatten.h"
#include "public/fpdf_ppo.h"
#include "public/fpdf_save.h"
#include "public/fpdfview.h"

namespace {

int FakeBlockWriter(FPDF_FILEWRITE* pThis,
                    const void* pData,
                    unsigned long size) {
  return size;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  FPDF_InitLibrary();

  FPDF_FILEWRITE writer;
  writer.version = 1;
  writer.WriteBlock = FakeBlockWriter;

  FPDF_DOCUMENT doc = FPDF_LoadMemDocument(data, size, nullptr);
  if (doc == nullptr)
    return 0;

  FPDF_DOCUMENT output_doc = FPDF_CreateNewDocument();
  FPDF_CopyViewerPreferences(output_doc, doc);
  FPDF_ImportPages(output_doc, doc, NULL, 0);
  FPDF_CloseDocument(doc);

  FPDF_SaveAsCopy(output_doc, &writer, 0);
  FPDF_CloseDocument(output_doc);

  FPDF_DestroyLibrary();
  return 0;
}
