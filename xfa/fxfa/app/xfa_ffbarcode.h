// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_XFA_FFBARCODE_H_
#define XFA_FXFA_APP_XFA_FFBARCODE_H_

#include "fxbarcode/BC_Library.h"
#include "xfa/fxfa/app/xfa_fftextedit.h"
#include "xfa/fxfa/cxfa_ffpageview.h"

class CXFA_FFBarcode : public CXFA_FFTextEdit {
 public:
  explicit CXFA_FFBarcode(CXFA_WidgetAcc* pDataAcc);
  ~CXFA_FFBarcode() override;

  // CXFA_FFTextEdit
  bool LoadWidget() override;
  void RenderWidget(CFX_Graphics* pGS,
                    CFX_Matrix* pMatrix,
                    uint32_t dwStatus) override;
  void UpdateWidgetProperty() override;
  bool OnLButtonDown(uint32_t dwFlags, const CFX_PointF& point) override;
  bool OnRButtonDown(uint32_t dwFlags, const CFX_PointF& point) override;
};

#endif  // XFA_FXFA_APP_XFA_FFBARCODE_H_
