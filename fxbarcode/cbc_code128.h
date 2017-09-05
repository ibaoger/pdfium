// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_CBC_CODE128_H_
#define FXBARCODE_CBC_CODE128_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/fx_dib.h"
#include "fxbarcode/cbc_onecode.h"

class CBC_OnedCode128Writer;

class CBC_Code128 : public CBC_OneCode {
 public:
  explicit CBC_Code128(BC_TYPE type);
  ~CBC_Code128() override;

  // CBC_OneCode:
  bool Encode(const CFX_WideStringC& contents) override;
  bool RenderDevice(CFX_RenderDevice* device,
                    const CFX_Matrix* matrix) override;
  BC_TYPE GetType() override;

  bool SetTextLocation(BC_TEXT_LOC loction);

 private:
  CBC_OnedCode128Writer* GetOnedCode128Writer();

  CFX_WideString m_renderContents;
};

#endif  // FXBARCODE_CBC_CODE128_H_
