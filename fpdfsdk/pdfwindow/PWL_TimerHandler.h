// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PDFWINDOW_PWL_TIMERHANDLER_H_
#define FPDFSDK_PDFWINDOW_PWL_TIMERHANDLER_H_

#include <memory>

#include "core/fxcrt/fx_basic.h"

class CFX_SystemHandler;
class CPWL_Timer;

class IPWL_TimerHandler {
 public:
  IPWL_TimerHandler();
  virtual ~IPWL_TimerHandler();

  void BeginTimer(int32_t nElapse);
  void EndTimer();
  virtual void TimerProc();
  virtual CFX_SystemHandler* GetSystemHandler() const = 0;

 private:
  std::unique_ptr<CPWL_Timer> m_pTimer;
};

#endif  // FPDFSDK_PDFWINDOW_PWL_TIMERHANDLER_H_
