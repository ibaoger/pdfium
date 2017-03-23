// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_CBC_QRCODE_H_
#define XFA_FXBARCODE_CBC_QRCODE_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/fx_dib.h"
#include "xfa/fxbarcode/cbc_codebase.h"

class CBC_QRCode : public CBC_CodeBase {
 public:
  CBC_QRCode();
  ~CBC_QRCode() override;

  // CBC_CodeBase:
  bool Encode(const CFX_WideStringC& contents,
              bool isDevice,
              int32_t& e) override;
  bool RenderDevice(CFX_RenderDevice* device,
                    const CFX_Matrix* matrix,
                    int32_t& e) override;
  bool RenderBitmap(CFX_RetainPtr<CFX_DIBitmap>& pOutBitmap,
                    int32_t& e) override;
  BC_TYPE GetType() override;

  bool SetVersion(int32_t version);
  bool SetErrorCorrectionLevel(int32_t level);
};

#endif  // XFA_FXBARCODE_CBC_QRCODE_H_
