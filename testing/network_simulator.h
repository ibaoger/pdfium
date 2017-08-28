// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_NETWORK_SIMULATOR_H_
#define TESTING_NETWORK_SIMULATOR_H_

#include <memory>

#include "core/fxcrt/cfx_unowned_ptr.h"
#include "public/fpdf_dataavail.h"
#include "public/fpdfview.h"
#include "testing/range_set.h"

class NetworkSimulator {
 public:
  explicit NetworkSimulator(FPDF_FILEACCESS* file_access);
  virtual ~NetworkSimulator();

  FPDF_FILEACCESS* GetFileAccess() const { return file_access_wrapper_.get(); }
  FX_FILEAVAIL* GetFileAvail() const { return file_avail_.get(); }
  FX_DOWNLOADHINTS* GetDownloadHints() const { return download_hints_.get(); }

  FPDF_BOOL IsDataAvail(size_t offset, size_t size);
  void AddSegment(size_t offset, size_t size);

  unsigned long GetFileSize();

  int GetBlock(unsigned long position, unsigned char* pBuf, unsigned long size);
  void Flush();
  void FlushWholeFile();

 private:
  CFX_UnownedPtr<FPDF_FILEACCESS> file_access_;
  std::unique_ptr<FPDF_FILEACCESS> file_access_wrapper_;
  std::unique_ptr<FX_FILEAVAIL> file_avail_;
  std::unique_ptr<FX_DOWNLOADHINTS> download_hints_;
  RangeSet available_data_;
  RangeSet requested_data_;
};

#endif  // TESTING_NETWORK_SIMULATOR_H_
